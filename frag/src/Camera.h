#ifndef FRAG_WEBCAM_H
#define FRAG_WEBCAM_H

// STL
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "types.h"
#include "Texture.h"

namespace frag {
    class Camera : public Texture {
        public:
            ~Camera();
            Camera(int device, double fps=0, cv::Size size=cv::Size(0,0));
            Camera(const std::string& path);

            void start();
            void stop();
            virtual void update() override;

            bool read(cv::OutputArray& out);

            int getHeight();
            int getWidth();

        private:
            void nextFrame(bool loop=true);

            std::mutex frame_mutex_;
            cv::Mat frame_;
            std::unique_ptr<cv::VideoCapture> vid_;
            std::thread thread_;
            std::atomic<bool> running_ = false;
            bool new_frame_ = false;
            int device_ = 0;
            cv::Size size_ = cv::Size(0, 0);
            double fps_ = 0;
            const std::string path_;
            std::chrono::high_resolution_clock::time_point last_frame_;

    };
}

#endif
