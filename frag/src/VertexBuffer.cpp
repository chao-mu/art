#include "VertexBuffer.h"

namespace frag {
    VertexBuffer::VertexBuffer(const void* data, unsigned int size) {
        // Copy position vetex attributes
        glGenBuffers(1, &glID_);
        glBindBuffer(GL_ARRAY_BUFFER, glID_);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &glID_);
    }

    void VertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, glID_);
    }

    void VertexBuffer::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}
