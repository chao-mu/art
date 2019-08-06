#ifndef FRAG_WEBCAM_H
#define FRAG_WEBCAM_H

// STL
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <condition_variable>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Texture.h"

namespace frag {
    class Video : public Texture {
        public:
            enum Playback {
                Mirror,
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

            void setReverse(bool t);

            void flipPlayback();

            static bool isVideo(const std::string& path);

        private:
            void next();
            void seek(int pos);
            std::pair<int, std::shared_ptr<cv::Mat>> readFrame();
            void signalWork();

            // Constructor Parameters
            const std::string path_;
            size_t buffer_size_;
            std::atomic<bool> reverse_ = false;
            bool auto_reset_;
            Playback playback_;

            std::optional<std::chrono::high_resolution_clock::time_point> last_update_;
            std::mutex buffer_mutex_;
            std::unique_ptr<cv::VideoCapture> vid_;
            std::thread thread_;
            std::atomic<bool> running_ = false;
            double fps_ = 0;

            std::atomic<int> last_frame_ = 0;
            int total_frames_ = 0;

            std::atomic<bool> requested_reset_ = false;

            int cursor_;
            std::vector<std::pair<int, std::shared_ptr<cv::Mat>>> buffer_;

            bool work_ready_ = false;
            std::mutex work_ready_mutex_;
            std::condition_variable work_ready_cv_;
    };
}

#endif
