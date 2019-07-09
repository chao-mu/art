#include "Camera.h"

// STL
#include <string>

namespace frag {
    Camera::Camera(int device, double fps, cv::Size size) : device_(device), size_(size), fps_(fps) {
    }

    Camera::Camera(const std::string& path) : path_(path) {
    }


    Camera::~Camera() {
        stop();
    }

    void Camera::update() {
        cv::Mat cam_image;
        if (read(cam_image)) {
            populate(cam_image);
        }
    }

    bool Camera::read(cv::OutputArray& out) {
        std::lock_guard guard(frame_mutex_);
        if (new_frame_) {
            new_frame_ = false;
            frame_.copyTo(out);
            return true;
        }

        return false;
    }

    void Camera::nextFrame(bool loop) {
        // Do nothing if it's not time to read a frame
        if (fps_ != 0) {
            std::chrono::high_resolution_clock::time_point now =
                std::chrono::high_resolution_clock::now();

            std::chrono::duration<float> duration = now - last_frame_;

            if (duration.count() < 1 / fps_) {
                return;
            }
        }

        cv::Mat frame;
        if (vid_->read(frame)) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            // Flip if capture device
            if (path_ == "") {
                flip(frame, frame, -1);
            } else {
                flip(frame, frame, 0);
            }

            frame_mutex_.lock();
            size_ = frame.size();
            frame_ = frame;
            new_frame_ = true;
            frame_mutex_.unlock();

            last_frame_ = std::chrono::high_resolution_clock::now();
        } else if (loop) {
            vid_->set(cv::CAP_PROP_POS_FRAMES, 0);
            nextFrame(false);
        }
    }

    void Camera::start() {
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

        // Ensure after start there's at least one frame available.
        nextFrame();

        running_ = true;
        thread_ = std::thread([this]{
            while (running_.load()) {
                nextFrame();
            }
        });
    }

    int Camera::getWidth() {
        return size_.width;
    }

    int Camera::getHeight() {
        return size_.height;
    }

    void Camera::stop() {
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }
}
