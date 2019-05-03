#include "Webcam.h"

#include <string>

Webcam::Webcam(int device, double fps, cv::Size size) : running_(false), new_frame_(false), device_(device), size_(size), fps_(fps) {
}

Webcam::~Webcam() {
    stop();
}

bool Webcam::read(cv::OutputArray& out) {
    std::lock_guard guard(frame_mutex_);
    if (new_frame_) {
        new_frame_ = false;
        frame_.copyTo(out);
        return true;
    }

    return false;
}

void Webcam::nextFrame() {
    cv::Mat frame;
    if (webcam_->read(frame)) {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        flip(frame, frame, -1);
        frame_mutex_.lock();
        size_ = frame.size();
        frame_ = frame;
        new_frame_ = true;
        frame_mutex_.unlock();
    }
}

Error Webcam::start() {
    if (running_.load()) {
        return {};
    }

    webcam_ = std::make_unique<cv::VideoCapture>(device_, cv::CAP_V4L2);
    if (!webcam_->isOpened()) {
        return "Unable to open capture device " + std::to_string(device_);
    }

    if (size_.width != 0) {
        std::cout << size_.width << std::endl;
        if (!webcam_->set(cv::CAP_PROP_FRAME_WIDTH, size_.width)) {
            return "Unable to set frame width";
        }

        if (!webcam_->set(cv::CAP_PROP_FRAME_HEIGHT, size_.height)) {
            return "Unable to set frame height";
        }
    }

    if (fps_ != 0) {
        if (!webcam_->set(cv::CAP_PROP_FPS, fps_)) {
            return "Unable to set frame rate";
        }
    }

    std::cout << "Camera FPS: " << webcam_->get(cv::CAP_PROP_FPS) << std::endl;


    // Ensure after start there's at least one frame available.
    nextFrame();

    running_ = true;
    thread_ = std::thread([this]{
        while (running_.load()) {
            nextFrame();
        }
    });

    return {};
}

int Webcam::getWidth() {
    return size_.width;
}

int Webcam::getHeight() {
    return size_.height;
}

void Webcam::stop() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}
