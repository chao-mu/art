#ifndef FRAG_VERTEXARRAY_H_
#define FRAG_VERTEXARRAY_H_

#include "GLUtil.h"

namespace frag {
    class VertexArray {
        public:
            VertexArray();
            ~VertexArray();

            void bind() const;
            void unbind() const;

        private:
            unsigned int glID_ = GL_FALSE;
    };
}

#endif
