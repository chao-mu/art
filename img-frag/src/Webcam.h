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
        Webcam(int device);
        Error start();
        void stop();
        bool read(cv::OutputArray& out);

        int getHeight();
        int getWidth();

        void setProp(int prop_id, double value);

    private:
        void nextFrame();

        std::mutex frame_mutex_;
        cv::Mat frame_;
        cv::VideoCapture webcam_;
        std::thread thread_;
        std::atomic<bool> running_;
        bool new_frame_;
        int device_;
        cv::Size size_;
};

#endif
