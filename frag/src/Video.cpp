#include "Video.h"

// STL
#include <algorithm>

namespace frag {
    Video::Video(int device, double fps, cv::Size size) : device_(device), size_(size), fps_(fps), buffer_size_(1) {
    }

    Video::Video(const std::string& path) : path_(path), buffer_size_(30) {
    }

    Video::~Video() {
        stop();
    }

    void Video::update() {
        if (fps_ != 0) {
            std::chrono::high_resolution_clock::time_point now =
                std::chrono::high_resolution_clock::now();

            std::chrono::duration<float> duration = now - last_frame_;

            if (duration.count() < 1 / fps_) {
                return;
            }
        }

        std::lock_guard guard(frame_mutex_);

        if (buffer_.empty()) {
            std::cerr << "WARNING: Video buffer empty! Try a video with a lower frame rate. Path:" <<
                path_ << std::endl;
            return;
        }

        if (!reverse_.load()) {
            populate(buffer_.back());
            buffer_.erase(buffer_.end());
        } else {
            populate(buffer_.front());
            buffer_.erase(buffer_.begin());
        }

        last_frame_ = std::chrono::high_resolution_clock::now();
    }

    void Video::nextChunk() {
        {
            std::lock_guard guard(frame_mutex_);

            if (buffer_.size() >= buffer_size_) {
                return;
            }
        }

        bool rev = reverse_.load();

        if (rev) {
            int pos = static_cast<int>(vid_->get(cv::CAP_PROP_POS_FRAMES));

            // If we were previously reversed account for that rewind...
            if (last_reverse_.load()) {
                pos -= reverse_chunk_size_;
            }

            pos -= reverse_chunk_size_;

            // Loop around if need be.
            if (pos < 0) {
                pos += frame_count_;
            }

            vid_->set(cv::CAP_PROP_POS_FRAMES, pos);
        }

        int chunk_size = rev ? reverse_chunk_size_ : 1;
        std::vector<cv::Mat> tmp_buf;

        for (int i = 0; i < chunk_size; i++) {
            cv::Mat frame;
            if (vid_->read(frame)) {
                cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
                // Flip if capture device
                if (path_ == "") {
                    flip(frame, frame, -1);
                } else {
                    flip(frame, frame, 0);
                }

                tmp_buf.push_back(frame);
            } else {
                vid_->set(cv::CAP_PROP_POS_FRAMES, 0);
            }
        }

        {
            std::lock_guard guard(frame_mutex_);

            size_ = tmp_buf.front().size();
            if (rev) {
                std::reverse(tmp_buf.begin(), tmp_buf.end());
            }

            buffer_.insert(buffer_.end(), tmp_buf.begin(), tmp_buf.end());
        }
        last_reverse_ = rev;
    }


    void Video::setReverse(bool t) {
        if (last_reverse_.load() != t) {
            std::lock_guard guard(frame_mutex_);
            std::reverse(buffer_.begin(), buffer_.end());
        }

        last_reverse_ = reverse_.load();
        reverse_ = t;
    }


    void Video::start() {
        if (running_.load()) {
            return;
        }

        if (path_ != "") {
            vid_ = std::make_unique<cv::VideoCapture>(path_);

            if (!vid_->isOpened()) {
                throw std::runtime_error("Unable to open video with path " + path_);
            }
        } else {
            vid_ = std::make_unique<cv::VideoCapture>(device_, cv::CAP_V4L2);

            if (!vid_->isOpened()) {
                throw std::runtime_error("Unable to open capture device " + std::to_string(device_));
            }
        }

        if (size_.width != 0) {
            std::cout << size_.width << std::endl;
            if (!vid_->set(cv::CAP_PROP_FRAME_WIDTH, size_.width)) {
                throw std::runtime_error("Unable to set frame width");
            }

            if (!vid_->set(cv::CAP_PROP_FRAME_HEIGHT, size_.height)) {
                throw std::runtime_error("Unable to set frame height");
            }
        }

        if (fps_ != 0) {
            if (!vid_->set(cv::CAP_PROP_FPS, fps_)) {
                throw std::runtime_error("Unable to set frame rate");
            }
        } else {
            fps_ = vid_->get(cv::CAP_PROP_FPS);
        }

        frame_count_ = static_cast<int>(vid_->get(cv::CAP_PROP_FRAME_COUNT));

        // Fill buffer
        if (path_ != "") {
            for (size_t i = 0; i < buffer_size_; i++) {
                nextChunk();
            }
        }

        running_ = true;
        thread_ = std::thread([this]{
            while (running_.load()) {
                nextChunk();
            }
        });
    }

    int Video::getWidth() {
        std::lock_guard guard(frame_mutex_);
        return size_.width;
    }

    int Video::getHeight() {
        std::lock_guard guard(frame_mutex_);
        return size_.height;
    }

    void Video::stop() {
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }
}
