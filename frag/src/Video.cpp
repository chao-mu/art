#include "Video.h"

// STL
#include <algorithm>

// Ours
#include "fileutil.h"

// #include "debug.h"
// #define debug_time true

#define WORK_THRESHOLD 3

namespace frag {
    /*
    Video::Video(int device, double fps, cv::Size size) : device_(device), size_(size), fps_(fps), buffer_size_(1) {
    }

    */

    Video::Video(const std::string& path, bool auto_reset, Playback pb) : path_(path), buffer_size_(30), reverse_(pb == Reverse), auto_reset_(auto_reset), playback_(pb) {
    }

    Video::~Video() {
        stop();
    }

    void Video::outFocus() {
        requested_reset_ = auto_reset_;
        last_update_.reset();
        signalWork();
    }

    void Video::signalWork() {
        std::lock_guard lk(work_ready_mutex_);
        work_ready_ = true;
        work_ready_cv_.notify_one();
    }

    void Video::update() {
        Media::update();

        std::chrono::high_resolution_clock::time_point now =
            std::chrono::high_resolution_clock::now();

        if (last_update_.has_value()) {
            std::chrono::duration<float> duration = now - last_update_.value();

            float frame_dur = 1 / static_cast<float>(fps_);
            // The amount we are past the next time we should display a frame
            float past_target = duration.count() - frame_dur;
            if (past_target < 0) {
                return;
            } else if (past_target > frame_dur) {
                std::cerr << "WARNING: Skipped a frame! Since last frame " << past_target << "ms has traspired in " << path_ << std::endl;
            }
        }

        std::lock_guard guard(buffer_mutex_);

        if (cursor_ < 0 || static_cast<size_t>(cursor_) >= buffer_.size()) {
            std::cerr << "WARNING: Video buffer empty! Try a video with a lower frame rate. Path:" <<
                path_ << std::endl;

            return;
        }

        std::pair<int, std::shared_ptr<cv::Mat>> frame = buffer_.at(cursor_);
        populate(*frame.second);

        //std::cout << path_ << " " << frame.first << std::endl;

        if (playback_ == Mirror) {
            if (frame.first >= last_frame_.load()) {
                reverse_ = true;
            } else if (frame.first == 0) {
                reverse_ = false;
            }
        }

        if (reverse_) {
            cursor_--;

            if (static_cast<float>(buffer_size_) * 0.5  - cursor_ > WORK_THRESHOLD) {
                signalWork();
            }
        } else {
            cursor_++;

            if (cursor_ - static_cast<float>(buffer_size_) * 0.5 > WORK_THRESHOLD) {
                signalWork();
            }
        }

        last_update_ = std::chrono::high_resolution_clock::now();
    }

    std::pair<int, std::shared_ptr<cv::Mat>> Video::readFrame() {
        int pos = static_cast<int>(vid_->get(cv::CAP_PROP_POS_FRAMES));
        auto ptr = std::make_shared<cv::Mat>();
        cv::Mat& frame = *ptr;
        if (vid_->read(frame)) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            // Flip if capture device
            if (path_ == "") {
                flip(frame, frame, -1);
            } else {
                flip(frame, frame, 0);
            }
        } else {
            last_frame_ = pos - 1;

            vid_->set(cv::CAP_PROP_POS_FRAMES, 0);

            return readFrame();
        }

        return std::make_pair(pos, ptr);
    }

    void Video::seek(int pos) {
        if (pos < 0) {
            pos += total_frames_;
        }

        pos = pos % total_frames_;

        vid_->set(cv::CAP_PROP_POS_FRAMES, pos);
    }

    void Video::next() {
        // At the end of the day, this is where we want the read cursor to end up.
        int middle = static_cast<int>(static_cast<double>(buffer_size_) * 0.5);

        // If we have a reset request, set the cursor to the start of the video
        // if it exists in our buffer.
        if (requested_reset_.load() && !buffer_.empty()) {
            std::lock_guard guard(buffer_mutex_);

            requested_reset_ = false;
            reverse_ = false;

            bool found = false;
            for (int i=0; i < static_cast<int>(buffer_size_); i++) {
                if (buffer_.at(i).first == 0) {
                    found = true;
                    cursor_ = i;
                    break;
                }
            }

            if (!found) {
                buffer_.clear();
            }
        }

        if (buffer_.empty()) {
            std::lock_guard guard(buffer_mutex_);

            // Start from half the buffersize before the end of the video.
            seek(-middle);
            for (int i=0; i < static_cast<int>(buffer_size_); i++) {
                std::pair<int, std::shared_ptr<cv::Mat>> frame = readFrame();
                if (frame.first == 0) {
                    cursor_ = i;
                }

                buffer_.push_back(frame);
            }

            return;
        }

        // Calculate current distance from center.
        // Overshooting and undershooting are non-issues.
        int diff, front_pos, back_pos;
        {
            std::lock_guard guard(buffer_mutex_);

            diff = cursor_ - middle;;
            front_pos = buffer_.front().first;
            back_pos = buffer_.back().first;
        }


        // Fill in order to make the current frame the center frame of the buffer.
        if (diff < 0) {
            // Absolute diff.
            diff *= -1;

            // Jump back by the amount we need to read to catch up.
            seek(front_pos - diff);

            std::vector<std::pair<int, std::shared_ptr<cv::Mat>>> tmp_buf;
            for (int i=0; i < diff; i++) {
                tmp_buf.push_back(readFrame());
            }

            {
                std::lock_guard guard(buffer_mutex_);
                buffer_.insert(buffer_.begin(), tmp_buf.begin(), tmp_buf.end());
                buffer_.erase(buffer_.end() - diff, buffer_.end());

                // Compensate for the current frame moving forwards.
                cursor_ += diff;
            }
        } else if (diff > 0) {
            seek(back_pos + 1);

            std::vector<std::pair<int, std::shared_ptr<cv::Mat>>> tmp_buf;
            for (int i=0; i < diff; i++) {
                tmp_buf.push_back(readFrame());
            }

            {
                std::lock_guard guard(buffer_mutex_);
                buffer_.insert(buffer_.end(), tmp_buf.begin(), tmp_buf.end());
                buffer_.erase(buffer_.begin(), buffer_.begin() + diff);

                // Compensate for the current frame moving backwards.
                cursor_ -= diff;
            }
        }
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

        // This is a guess, apparently it can be wrong
        total_frames_ = static_cast<int>(vid_->get(cv::CAP_PROP_FRAME_COUNT));
        if (total_frames_ < 0) {
            throw std::runtime_error("Unable to accurately determine number of frames for " + path_);
        }

        last_frame_ = total_frames_ - 1;

        fps_ = vid_->get(cv::CAP_PROP_FPS);
        if (fps_ <= 0) {
            throw std::runtime_error("Unable to accurately determine number FPS for " + path_);
        }

        next();

        running_ = true;
        thread_ = std::thread([this] {
            //DEBUG_TIME_DECLARE(work_wait)
            while (running_.load()) {
                {
                    //DEBUG_TIME_START(work_wait)
                    std::unique_lock<std::mutex> lk(work_ready_mutex_);
                    work_ready_cv_.wait(lk, [this]{return work_ready_ || !running_.load();});
                    //DEBUG_TIME_END(work_wait)
                }

                if (!running_.load()) {
                    break;
                }

                next();

                {
                    std::lock_guard lk(work_ready_mutex_);
                    work_ready_ = false;
                }
            }
        });
    }

    void Video::flipPlayback() {
        setReverse(!reverse_);
    }

    void Video::setReverse(bool t) {
        reverse_ = t;
    }

    void Video::stop() {
        running_ = false;
        {
            std::lock_guard guard(work_ready_mutex_);
            work_ready_cv_.notify_one();
        }
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
