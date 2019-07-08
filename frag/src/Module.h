#ifndef FRAG_MODULE_H_
#define FRAG_MODULE_H_

// STL
#include <string>

// Ours
#include "GLUtil.h"
#include "ShaderProgram.h"
#include "PingPongTexture.h"
#include "types.h"

namespace frag {
    class Module {
        public:
            struct Source {
                std::string name;
                char first = 'r';
                char second = 'g';
                char third = 'b';
                char fourth = 'a';
                float amp = 1;
                float shift = 0;
                bool is_texture = false;
            };

            static std::string toChannelName(const std::string name);

            Module(const std::string& output, const std::string& path, const Resolution& res);

            void addSource(const std::string& input, Source src);
            std::map<std::string, std::string> getTextureSources() const;
            const std::string& getOutput() const;
            const std::string& getPath() const;
            void compile();
            std::shared_ptr<ShaderProgram> getShaderProgram();
            void bind();
            void unbind();
            GLuint getFBO();
            GLenum getReadableBuf();
            std::shared_ptr<Texture> getLastOutTex();

        private:
            const std::string output_;
            const std::string path_;
            std::map<std::string, Source> sources_;
            std::shared_ptr<PingPongTexture> ping_pong_;
            GLuint fbo_;
            const Resolution resolution_;
            std::shared_ptr<ShaderProgram> program_;
    };
}

#endif
