#include "ShaderProgram.h"

// STL

// OpenGL
#include <GLFW/glfw3.h>

// ours
#include "fileutil.h"

namespace frag {

    // Start with a valid program
    ShaderProgram::ShaderProgram() : program_(glCreateProgram()) {}

    // Clean up the shaders and programs we created
    ShaderProgram::~ShaderProgram() {
        for (const auto& shader : shaders_) {
            glDeleteShader(shader);
        }

        glDeleteProgram(program_);
    }

    void ShaderProgram::bind() {
        glUseProgram(program_);
    }

    void ShaderProgram::unbind() {
        glUseProgram(0);
    }

    void ShaderProgram::loadShaderStr(GLenum type, const std::string& source, const std::string& path) {
        const char* c_source = source.c_str();
        GLuint handle = glCreateShader(type);

        glShaderSource(handle, 1, &c_source, NULL);
        glCompileShader(handle);

        int status = 0;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
        if (!status) {
            GLint log_length = 0;
            glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
            std::vector<char> v(static_cast<size_t>(log_length));
            glGetShaderInfoLog(handle, log_length, NULL, v.data());

            throw std::runtime_error("Error compiling " + path +  ":\n" + std::string(begin(v), end(v)));
        }

        shaders_.push_back(handle);
    }

    void ShaderProgram::loadShader(GLenum type, const std::string& path) {
        loadShaderStr(type, fileutil::slurp(path), path);
    }

    void ShaderProgram::markUniformInUse(const std::string& name) {
        std::optional<GLint> loc = getUniformLoc(name);
        if (loc) {
            set_uniforms_.push_back(loc.value());
        }
    }

    void ShaderProgram::setUniform(const std::string& name, std::function<void(GLint&)> f) {
        if (!uniforms_.count(name)) {
            return;
        }

        GLint id = uniforms_.at(name);
        f(id);

        set_uniforms_.push_back(id);
    }

    std::vector<std::string> ShaderProgram::getUnsetUniforms() {
        std::vector<std::string> unset;

        for (const auto& kv : uniforms_) {
            std::string name = kv.first;
            GLint id = kv.second;

            if (set_uniforms_.end() == std::find(set_uniforms_.begin(), set_uniforms_.end(), id)) {
                unset.push_back(name);
            }
        }

        return unset;
    }

    void ShaderProgram::compile() {
        glDeleteProgram(program_);
        program_ = glCreateProgram();
        for (const auto& shader : shaders_) {
            glAttachShader(program_, shader);
        }

        glLinkProgram(program_);

        int status = 0;
        glGetProgramiv(program_, GL_LINK_STATUS, &status);
        if (!status) {
            GLint log_length = 0;
            glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_length);
            std::vector<char> v(static_cast<size_t>(log_length));
            glGetProgramInfoLog(program_, log_length, NULL, v.data());

            throw std::runtime_error("Error linking shader program: \n" + std::string(begin(v), end(v)));
        }

        GLint uni_name_len = 0;
        glGetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uni_name_len);

        GLint count;
        glGetProgramiv(program_, GL_ACTIVE_UNIFORMS, &count);
        for (GLuint i = 0; i < (GLuint)count; i++) {
            std::vector<GLchar> name(static_cast<size_t>(uni_name_len));

            GLsizei length;
            GLint size;
            GLenum type;
            glGetActiveUniform(program_, i, uni_name_len, &length, &size, &type, &name[0]);

            GLint loc = glGetUniformLocation(program_, name.data());
            uniforms_[std::string(name.data())] = loc;
            uniform_types_[std::string(name.data())] = type;
        }
    }

    std::optional<GLenum> ShaderProgram::getUniformType(const std::string& name) {
        if (!uniform_types_.count(name)) {
            return {};
        }

        return uniform_types_.at(name);
    }

    std::optional<GLint> ShaderProgram::getUniformLoc(const std::string& name) {
        if (!uniforms_.count(name)) {
            return {};
        }

        return uniforms_.at(name);
    }
}
