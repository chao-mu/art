#ifndef WEBCAM_H
#define WEBCAM_H

#include <mutex>
#include <atomic>
#include <thread>

#include <opencv2/opencv.hpp>

#include "types.h"

class Webcam {
    public:
        ~Webcam();
        Webcam(int device, double fps=0, cv::Size size=cv::Size(0,0));
        Error start();
        void stop();
        bool read(cv::OutputArray& out);

        int getHeight();
        int getWidth();

    private:
        void nextFrame();

        std::mutex frame_mutex_;
        cv::Mat frame_;
        std::unique_ptr<cv::VideoCapture> webcam_;
        std::thread thread_;
        std::atomic<bool> running_;
        bool new_frame_;
        int device_;
        cv::Size size_;
        double fps_;
};

#endif
