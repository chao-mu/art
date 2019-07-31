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
            enum Playback {
                PingPong,
                Forward,
                Reverse
            };

            ~Video();
            // Video(int device, double fps=0, cv::Size size=cv::Size(0,0));
            Video(const std::string& path, bool auto_reset, Playback pb=Forward);

            void start();
            void stop();
            virtual void update() override;

            virtual void outFocus() override;

            int getHeight();
            int getWidth();

            void setReverse(bool t);

            void flipPlayback();

        private:
            void nextChunk();

            void seek(int pos);

            std::mutex buffer_mutex_;
            cv::Mat frame_;
            std::unique_ptr<cv::VideoCapture> vid_;
            std::thread thread_;
            std::atomic<bool> running_ = false;
            int device_ = 0;
            cv::Size size_ = cv::Size(0, 0);
            double fps_ = 0;
            const std::string path_;
            std::chrono::high_resolution_clock::time_point last_frame_;

            std::vector<std::pair<int, std::shared_ptr<cv::Mat>>> buf_a_;
            std::vector<std::pair<int, std::shared_ptr<cv::Mat>>> buf_b_;
            size_t buffer_size_;
            std::atomic<bool> reverse_ = false;
            const int reverse_chunk_size_ = 5;
            int buffer_idx_ = 0;
            int frame_count_ = 0;
            bool auto_reset_;
            Playback playback_;

            std::atomic<bool> requested_reverse_ = false;
            std::atomic<bool> requested_reset_ = false;
    };
}

#endif
