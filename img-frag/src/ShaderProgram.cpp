#include "ShaderProgram.h"

#include <fstream>
#include <errno.h>
#include <cstring>
#include <sstream>
#include <iostream>

#include <GLFW/glfw3.h>

using Error = std::optional<std::string>;

// Start with a valid program
ShaderProgram::ShaderProgram() : program_(glCreateProgram()) {}

// Clean up the shaders and programs we created
ShaderProgram::~ShaderProgram() {
    for (const auto& kv : shaders_) {
        glDeleteShader(kv.second.handle);
    }

    glDeleteProgram(program_);
}

void ShaderProgram::registerShader(
    GLenum type,
    const std::string& path,
    GLuint handle
) {
    if (shaders_.count(type) && shaders_.at(type).handle != GL_FALSE) {
        glDeleteShader(shaders_[type].handle);
    }

    shaders_[type] = Shader{};
    Shader& shader = shaders_.at(type);
    shader.handle = handle;
    shader.path = path;
    shader.type = type;

    try {
        shader.last_modified = boost::filesystem::last_write_time(path);
    } catch (boost::filesystem::filesystem_error& err) {
        shader.last_modified = 0;
    }

    should_switch_ = true;
}

Error ShaderProgram::loadShader(GLenum type, const std::string& path) {
    std::ifstream ifs(path);
    if (ifs.fail()) {
        std::ostringstream err;
        err << "Error loading " << path << " - " <<  std::strerror(errno);
        return err.str();
    }

    std::stringstream stream;
    stream << ifs.rdbuf();
    const std::string source = stream.str();

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

        return "Error compiling " + path +  ":\n" + std::string(begin(v), end(v));
    }

    registerShader(type, path, handle);

    return {};
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

Error ShaderProgram::update() {
    for (const auto& kv : shaders_) {
        const Shader& shader = kv.second;
        const std::string path = shader.path.string();
        std::time_t last_modified = 0;
        try {
            last_modified = boost::filesystem::last_write_time(path);
        } catch (boost::filesystem::filesystem_error& err) {
            return err.what();
        }

        boost::filesystem::last_write_time(path);
        if (last_modified > shader.last_modified || shader.handle == GL_FALSE) {
            Error err = loadShader(kv.first, path);
            if (err.has_value()) {
                return "Error loading " + path + ":\n" + err.value();
            }
        }
    }

    if (should_switch_) {
        GLuint next_prog = glCreateProgram();
        for (const auto& kv : shaders_) {
            glAttachShader(next_prog, kv.second.handle);
        }

        glLinkProgram(next_prog);

        int status = 0;
        glGetProgramiv(next_prog, GL_LINK_STATUS, &status);
        if (!status) {
            GLint log_length = 0;
            glGetProgramiv(next_prog, GL_INFO_LOG_LENGTH, &log_length);
            std::vector<char> v(static_cast<size_t>(log_length));
            glGetProgramInfoLog(next_prog, log_length, NULL, v.data());

            return "Error linking shader program: \n" + std::string(begin(v), end(v));
        }

        GLint uni_name_len = 0;
        glGetProgramiv(next_prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uni_name_len);

        GLint count;
        glGetProgramiv(next_prog, GL_ACTIVE_UNIFORMS, &count);
        uniforms_.clear();
        for (GLuint i = 0; i < (GLuint)count; i++) {
            std::vector<GLchar> name(static_cast<size_t>(uni_name_len));

            GLsizei length;
            GLint size;
            GLenum type;
            glGetActiveUniform(next_prog, i, uni_name_len, &length, &size, &type, &name[0]);

            GLint loc = glGetUniformLocation(next_prog, name.data());
            uniforms_[std::string(name.data())] = loc;
        }

        // Change out the program
        glDeleteProgram(program_);
        program_ = next_prog;

        should_switch_ = false;
    }

    set_uniforms_.clear();

    return {};
}

std::unordered_set<std::string> ShaderProgram::getWarnings() {
    std::unordered_set<std::string> warnings;

    for (const auto& unset : getUnsetUniforms()) {
        warnings.insert("in-use uniform '" + unset + "' is unset.");
    }

    return warnings;
}

std::optional<GLint> ShaderProgram::getUniformLoc(std::string name) {
    if (!uniforms_.count(name)) {
        return {};
    }

    return uniforms_.at(name);
}

GLuint ShaderProgram::getProgram() {
    return program_;
}
