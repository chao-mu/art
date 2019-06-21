#ifndef FRAG_GLUTIL_H_
#define FRAG_GLUTIL_H_

// STL
#include <stdlib.h>

// STL
#include <stdexcept>

// OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLCall(x) \
    while (glGetError() != GL_NO_ERROR); \
    x; \
    if (!frag::GLLogCall(#x, __FILE__, __LINE__)) throw std::runtime_error("OpenGL Error");

    //if (!GLLogCall(#x, __FILE__, __LINE__)) exit(EXIT_FAILURE);


namespace frag {
    bool GLLogCall(const char* function, const char* file, int line);
}

#endif
