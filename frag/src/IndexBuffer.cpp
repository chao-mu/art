#include "IndexBuffer.h"

namespace frag {
    IndexBuffer::IndexBuffer(const void* data, unsigned int count) : count_(count) {
        // Copy position vetex attributes
        glGenBuffers(1, &glID_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glID_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
    }

    IndexBuffer::~IndexBuffer() {
        glDeleteBuffers(1, &glID_);
    }

    void IndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glID_);
    }

    void IndexBuffer::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    unsigned int IndexBuffer::getCount() const {
        return count_;
    }
}
