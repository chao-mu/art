#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

// STL
#include <map>
#include <functional>
#include <unordered_set>

// Boost
#include <boost/filesystem.hpp>

// OpenGL
#include <GL/glew.h>

class ShaderProgram {
    public:
        // Represents each shader and meta data needed for hotloading
        struct Shader {
            GLuint handle;
            std::time_t last_modified;
            boost::filesystem::path path;
            GLenum type;
        };

        ShaderProgram();
        ~ShaderProgram();

        // Execute this regularly to load new changes and maintain program id
        std::optional<std::string> update();

        // The shader program id
        GLuint getProgram();

        // Retrieves a uniform's location
        std::optional<GLint> getUniformLoc(std::string name);

        // Sets a uniform and marks it as being in use
        void setUniform(const std::string& name, std::function<void(GLint&)> f);

        // Marks a uniform as being in use to prevent a warning of it not being in use
        void markUniformInUse(const std::string& name);

        // Retrieve all the uniform names of uniforms we haven't set
        std::vector<std::string> getUnsetUniforms();

        // Retrieve warnings
        std::unordered_set<std::string> getWarnings();

        // Register a shader. Will cause program to be reloaded next update.
        void registerShader(
            GLenum type,
            const std::string& path,
            GLenum handle=GL_FALSE
        );

    private:
        std::map<GLenum, Shader> shaders_;
        std::map<std::string, GLint> uniforms_;
        std::vector<GLint> set_uniforms_;
        bool should_switch_ = false;
        GLuint program_;

        // Attempt to load shader of the given type into this program
        std::optional<std::string> loadShader(GLenum type, const std::string& path);
};

#endif
