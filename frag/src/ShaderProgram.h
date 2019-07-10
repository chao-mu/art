#ifndef FRAG_SHADER_PROGRAM_H
#define FRAG_SHADER_PROGRAM_H

// STL
#include <map>
#include <functional>
#include <unordered_set>

// Boost
#include <boost/filesystem.hpp>

// OpenGL
#include <GL/glew.h>

// Ours
#include "midi/Control.h"

namespace frag {
    class ShaderProgram {
        public:
            ShaderProgram();
            ~ShaderProgram();

            // Execute this regularly to load new changes and maintain program id
            void compile();

            void bind();
            void unbind();

            // Attempt to load shader of the given type into this program
            void loadShader(GLenum type, const std::string& path);

            // Retrieves a uniform's location
            std::optional<GLint> getUniformLoc(const std::string& name);

            // Retrieves a uniform's type
            std::optional<GLenum> getUniformType(const std::string& name);


            // Sets a uniform and marks it as being in use
            void setUniform(const std::string& name, std::function<void(GLint&)> f);
            void setUniform(const std::string& name, const midi::Control& v);
            void setUniform(const std::string& name, bool v);
            void setUniform(const std::string& name, float v);
            void setUniform(const std::string& name, std::vector<float> in_v);

            // Marks a uniform as being in use to prevent a warning of it not being in use
            void markUniformInUse(const std::string& name);

            // Retrieve all the uniform names of uniforms we haven't set
            std::vector<std::string> getUnsetUniforms();

            // Load a shader from a string
            void loadShaderStr(GLenum type, const std::string& source, const std::string& path);

        private:
            std::map<std::string, GLint> uniforms_;
            std::map<std::string, GLenum> uniform_types_;
            std::vector<GLint> set_uniforms_;
            GLuint program_;
            std::vector<GLuint> shaders_;
    };
}

#endif
