#include "Module.h"

// STL
#include <sstream>
#include <regex>

namespace frag {
    Module::Module(
            const std::string& output,
            const std::string& type,
            const Resolution& res
        ) : output_(output), type_(type), resolution_(res) , program_(std::make_shared<ShaderProgram>()) {

        // Our render target
        ping_pong_ = std::make_shared<frag::PingPongTexture>(
                GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1);

        // Initialize with blank images
        for (const auto& tex : {ping_pong_->getSrcTex(), ping_pong_->getDestTex()}) {
            tex->bind();
            GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution_.width, resolution_.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
            tex->unbind();
        }

        // Create and bind the frame buffer we will be rendering to
        GLCall(glGenFramebuffers(1, &fbo_));

        // Bind the FBO in order to then associate texture's with it
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));

        // Bind the textures to the frame buffer
        GLCall(glFramebufferTexture(
            GL_FRAMEBUFFER,
            ping_pong_->getSrcDrawBuf(),
            ping_pong_->getSrcTex()->getID(),
            0
        ));

        GLCall(glFramebufferTexture(
            GL_FRAMEBUFFER,
            ping_pong_->getDestDrawBuf(),
            ping_pong_->getDestTex()->getID(),
            0
        ));
    }

    GLenum Module::getReadableBuf() {
        return ping_pong_->getSrcDrawBuf();
    }

    GLuint Module::getFBO() {
        return fbo_;
    }

    std::shared_ptr<Texture> Module::getLastOutTex() {
        return ping_pong_->getSrcTex();
    }

    void Module::unbind() {
        program_->unbind();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ping_pong_->swap();
    }

    void Module::bind() {
        // Create and bind the frame buffer we will be rendering to
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
        program_->bind();
        GLCall(glDrawBuffer(ping_pong_->getDestDrawBuf()));
    }

    void Module::addTextureSource(const std::string& input, Source src) {
        texture_sources_[input] = src;
    }

    std::map<std::string, std::string> Module::getTextureSources() const {
        std::map<std::string, std::string> sources;
        for (const auto& kv : texture_sources_) {
            sources[kv.first] = kv.second.name;
        }

        return sources;
    }

    const std::string& Module::getOutput() const {
        return output_;
    }

    const std::string& Module::getType() const {
        return type_;
    }

    std::shared_ptr<ShaderProgram> Module::getShaderProgram() {
        return program_;
    }

    void Module::compile() {
        std::string vert_shader = R"V(
            #version 410

            out vec2 texcoord;
            out vec2 texcoordL;
            out vec2 texcoordR;
            out vec2 texcoordT;
            out vec2 texcoordTL;
            out vec2 texcoordTR;
            out vec2 texcoordB;
            out vec2 texcoordBL;
            out vec2 texcoordBR;
            out vec2 uv;

            uniform vec2 iResolution;

            layout (location = 0) in vec3 aPos;

            void main() {
                gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);

                float widthStep = 1. / iResolution.x;
                float heightStep = 1. / iResolution.y;

                vec2 st = aPos.xy * .5 + .5;
                uv = st - .5;
                uv.x *= iResolution.x / iResolution.y;

                texcoord = st;
                texcoordL = st + vec2(-widthStep, 0);
                texcoordR = st + vec2(widthStep, 0);

                texcoordT = st + vec2(0, heightStep);
                texcoordTL = st + vec2(-widthStep, heightStep);
                texcoordTR = st + vec2(widthStep, heightStep);

                texcoordB = st + vec2(0, -heightStep);
                texcoordBL = st + vec2(-widthStep, -heightStep);
                texcoordBR = st + vec2(widthStep, -heightStep);
            }
        )V";

        const std::string shader_path = getType() + ".glsl";

        std::ifstream ifs(shader_path);
        if (ifs.fail()) {
            std::ostringstream err;
            err << "Error loading " << shader_path << " - " <<  std::strerror(errno);
            throw std::runtime_error(err.str());
        }

        std::stringstream frag_shader;
        const std::regex pragma_channel_re(R"(^#pragma\s+channel\s+(.*)$)");
        const std::regex channel_info_re(R"(^(\w+)\s+(\w+)\s*$)");
        std::smatch match;

        std::string line;
        while (std::getline(ifs, line)) {
            if (std::regex_match(line, match, pragma_channel_re)) {
                std::string channel_info = match[1].str();
                if (std::regex_match(channel_info, match, channel_info_re)) {
                    const std::string type = match[1];
                    const std::string name = match[2];
                    bool is_texture = texture_sources_.count(name) > 0;

                    if (is_texture) {
                        frag_shader << "uniform sampler2D " << name << ";\n";
                    } else {
                        frag_shader << "uniform " << type << " " << name << ";\n";
                    }

                    frag_shader << type << " channel_" << name << "(in vec2 uv) {\n";
                    if (is_texture) {
                        Source& src = texture_sources_.at(name);
                        frag_shader << "   return " << "texture(" << name << ", uv)";
                        if (type == "float") {
                            frag_shader << "." << src.first;
                        } else if (type == "vec2") {
                            frag_shader << "." << src.first << src.second;
                        } else if (type == "vec3") {
                            frag_shader << "." << src.first << src.second << src.third;
                        } else if (type == "vec4") {
                            frag_shader << "." << src.first << src.second << src.third << src.fourth;
                        } else if (type == "bool") {
                            frag_shader << src.first << " > 0.5";
                        } else {
                            throw std::runtime_error("unsupported channel type '" + type + "'");
                        }

                        frag_shader << ";\n";
                    } else {
                        frag_shader << "  return " << name << ";\n";
                    }
                    frag_shader << "}\n";
                } else {
                    throw std::runtime_error("Unable to parse channel definition line: " + line);
                }
            } else {
                frag_shader << line << "\n";
            }
        }

        program_->loadShaderStr(GL_VERTEX_SHADER, vert_shader, "internal-vert.glsl");
        program_->loadShaderStr(GL_FRAGMENT_SHADER, frag_shader.str(), shader_path);
        program_->compile();
    }
}
