#ifndef FRAG_VERTEXBUFFER_H_
#define FRAG_VERTEXBUFFER_H_

#include "GLUtil.h"

namespace frag {
    class VertexBuffer {
        public:
            VertexBuffer(const void* data, unsigned int size);
            ~VertexBuffer();

            void bind() const;
            void unbind() const;

        private:
            unsigned int glID_ = 0;
    };
}

#endif
