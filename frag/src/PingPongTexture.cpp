#include "PingPongTexture.h"

#define SRC 0
#define DEST 1

namespace frag {
    PingPongTexture::PingPongTexture(GLenum src, GLenum dest) {
        textures_[SRC] = std::make_shared<Texture>();
        textures_[DEST] = std::make_shared<Texture>();

        draw_bufs_[SRC] = src;
        draw_bufs_[DEST] = dest;
    }

    void PingPongTexture::swap() {
        std::swap(draw_bufs_[SRC], draw_bufs_[DEST]);
        std::swap(textures_[SRC], textures_[DEST]);
    }

    std::shared_ptr<Texture> PingPongTexture::getSrcTex() {
        return textures_[SRC];
    }

    std::shared_ptr<Texture> PingPongTexture::getDestTex() {
        return textures_[DEST];
    }

    GLuint PingPongTexture::getSrcDrawBuf() const {
        return draw_bufs_[SRC];
    }

    GLuint PingPongTexture::getDestDrawBuf() const {
        return draw_bufs_[DEST];
    }
}
