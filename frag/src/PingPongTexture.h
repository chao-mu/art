#ifndef FRAG_PINGPONGTEXTURE_H_
#define FRAG_PINGPONGTEXTURE_H_

// STL
#include <memory>

// Ours
#include "Texture.h"
#include "GLUtil.h"

namespace frag {
    class PingPongTexture {
        public:
            PingPongTexture(GLenum src, GLenum dest);
            void swap();

            std::shared_ptr<Texture> getSrcTex();
            std::shared_ptr<Texture> getDestTex();
            GLuint getSrcDrawBuf() const;
            GLuint getDestDrawBuf() const;

        private:
            std::shared_ptr<Texture> textures_[2];
            GLuint draw_bufs_[2];
    };
}

#endif
