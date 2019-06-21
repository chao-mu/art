#include "VertexArray.h"

namespace frag {
    VertexArray::VertexArray() {
        glGenVertexArrays(1, &glID_);
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &glID_);
    }

    void VertexArray::bind() const {
        glBindVertexArray(glID_);
    }

    void VertexArray::unbind() const {
        glBindVertexArray(0);
    }
}
