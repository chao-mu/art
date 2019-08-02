#include "Video.h"

// STL
#include <algorithm>

// Ours
#include "fileutil.h"

// TODO: Clean this all up with inspiration from the actor model

namespace frag {
    /*
    Video::Video(int device, double fps, cv::Size size) : device_(device), size_(size), fps_(fps), buffer_size_(1) {
    }

    */

    Video::Video(const std::string& path, bool auto_reset, Playback pb) : path_(path), buffer_size_(30), auto_reset_(auto_reset), playback_(pb) {
    }

    Video::~Video() {
        stop();
    }

    void Video::outFocus() {
        requested_reset_ = auto_reset_;
    }

    void Video::update() {
        Media::update();

        std::chrono::high_resolution_clock::time_point now =
            std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> duration = now - last_frame_;

        if (duration.count() < 1 / fps_) {
            return;
        }

        std::lock_guard guard(buffer_mutex_);

        if (buf_a_.empty()) {
            std::cerr << "WARNING: Video buffer empty! Try a video with a lower frame rate. Path:" <<
                path_ << std::endl;
            return;
        }

        populate(*buf_a_.front().second.get());
        buf_b_.insert(buf_b_.begin(), buf_a_.front());
        buf_a_.erase(buf_a_.begin());

        if (buf_b_.size() > buffer_size_) {
            buf_b_.pop_back();
        }

        last_frame_ = std::chrono::high_resolution_clock::now();
    }


    void Video::seek(int pos) {
        if (pos < 0) {
            pos += frame_count_;
        }

        pos = pos % frame_count_;

        vid_->set(cv::CAP_PROP_POS_FRAMES, pos);
    }

    void Video::nextChunk() {
        if (requested_reset_.load()) {
            std::lock_guard guard(buffer_mutex_);

            buf_a_.clear();
            buf_b_.clear();

            vid_->set(cv::CAP_PROP_POS_FRAMES, 0);

            requested_reverse_ = false;
            requested_reset_ = false;
        }

        if (buf_a_.size() >= buffer_size_) {
            return;
        }

        bool last_rev = reverse_.load();
        bool rev = requested_reverse_.load();
        reverse_ = rev;

        if (last_rev != rev) {
            std::lock_guard guard(buffer_mutex_);
            std::swap(buf_a_, buf_b_);
            buf_b_.clear();
        }

        if (rev) {
            int pos;
            {
                std::lock_guard guard(buffer_mutex_);
                pos = buf_a_.back().first - reverse_chunk_size_;
            }

            seek(pos);
        } else if (last_rev) {
            int pos;
            {
                std::lock_guard guard(buffer_mutex_);
                pos = buf_a_.back().first + 1;
            }

            seek(pos);
        }

        int chunk_size = rev ? reverse_chunk_size_ : 1;
        std::vector<std::pair<int, std::shared_ptr<cv::Mat>>> tmp_buf;

        for (int i = 0; i < chunk_size; i++) {
            int pos = static_cast<int>(vid_->get(cv::CAP_PROP_POS_FRAMES));

            auto ptr = std::make_shared<cv::Mat>();
            cv::Mat& frame = *ptr.get();
            if (vid_->read(frame)) {
                cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
                // Flip if capture device
                if (path_ == "") {
                    flip(frame, frame, -1);
                } else {
                    flip(frame, frame, 0);
                }

                tmp_buf.push_back(std::make_pair(pos, ptr));
            } else {
                vid_->set(cv::CAP_PROP_POS_FRAMES, 0);
                i--;
            }
        }

        if (rev) {
            std::reverse(tmp_buf.begin(), tmp_buf.end());
        }

        {
            std::lock_guard guard(buffer_mutex_);

            buf_a_.insert(buf_a_.end(), tmp_buf.begin(), tmp_buf.end());

            size_ = tmp_buf.front().second->size();
        }
    }


    void Video::setReverse(bool t) {
        requested_reverse_ = t;
    }


    void Video::start() {
        if (running_.load()) {
            return;
        }

        vid_ = std::make_unique<cv::VideoCapture>(path_);

        if (!vid_->isOpened()) {
            throw std::runtime_error("Unable to open video with path " + path_);
        }

        /*
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
        */

        frame_count_ = static_cast<int>(vid_->get(cv::CAP_PROP_FRAME_COUNT));
        fps_ = vid_->get(cv::CAP_PROP_FPS);

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
        std::lock_guard guard(buffer_mutex_);
        return size_.width;
    }

    int Video::getHeight() {
        std::lock_guard guard(buffer_mutex_);
        return size_.height;
    }

    void Video::flipPlayback() {
        setReverse(!reverse_);
    }

    void Video::stop() {
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    bool Video::isVideo(const std::string& path) {
        const std::string exts[] = {
            "3g2", "3gp", "aaf", "asf", "avchd", "avi", "drc", "flv", "m2v",
            "m4p", "m4v", "mkv", "mng", "mov", "mp2", "mp4", "mpe", "mpeg", "mpg",
            "mpv", "mxf", "nsv", "ogg", "ogv", "qt", "rm", "rmvb", "roq", "svi",
            "vob", "webm", "wmv", "yuv"
        };

        for (const auto& ext : exts) {
            if (fileutil::hasExtension(path, ext)) {
                return true;
            }
        }

        return false;
    }
}
