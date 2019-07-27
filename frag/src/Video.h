#ifndef FRAG_WEBCAM_H
#define FRAG_WEBCAM_H

// STL
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Texture.h"

namespace frag {
    class Video : public Texture {
        public:
            ~Video();
            Video(int device, double fps=0, cv::Size size=cv::Size(0,0));
            Video(const std::string& path);

            void start();
            void stop();
            virtual void update() override;

            int getHeight();
            int getWidth();

            void setReverse(bool t);

            void flipPlayback();

        private:
            void nextChunk();

            std::mutex frame_mutex_;
            cv::Mat frame_;
            std::unique_ptr<cv::VideoCapture> vid_;
            std::thread thread_;
            std::atomic<bool> running_ = false;
            int device_ = 0;
            cv::Size size_ = cv::Size(0, 0);
            double fps_ = 0;
            const std::string path_;
            std::chrono::high_resolution_clock::time_point last_frame_;

            std::vector<cv::Mat> buffer_;
            size_t buffer_size_;
            std::atomic<bool> reverse_ = false;
            std::atomic<bool> last_reverse_ = false;
            const int reverse_chunk_size_ = 5;
            int buffer_idx_ = 0;
            int frame_count_ = 0;
    };
}

#endif
