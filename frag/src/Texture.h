#ifndef FRAG_TEXTURE_H_
#define FRAG_TEXTURE_H_

// STL
#include <functional>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "GLUtil.h"
#include "Media.h"

namespace frag {
    class Texture : public Media {
        public:
            Texture();
            ~Texture();

            void save(const std::string& path);
            void bind(unsigned int slot = 0) override;
            void unbind() override;
            void populate(GLint internal_format, GLsizei width, GLsizei height,
                    GLenum format, GLenum type, const GLvoid * data);
            void populate(cv::Mat& frame);
            GLuint getID() const;

        private:
            void borrowBind(std::function<void()> f);

            unsigned int glID_ = 0;
    };
}

#endif
