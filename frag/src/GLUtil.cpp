#include "GLUtil.h"

// STL
#include <iostream>

namespace frag {
    bool GLLogCall(const char* function, const char* file, int line) {
        while (GLenum err = glGetError()) {
            std::cerr << "[OpenGL Error] (" << err << "): " << function <<
                " " << file << ": " << line << std::endl;

            return false;
        }

        return true;
    }
}
