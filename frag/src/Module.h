#ifndef FRAG_MODULE_H_
#define FRAG_MODULE_H_

// STL
#include <string>
#include <variant>

// Ours
#include "GLUtil.h"
#include "ShaderProgram.h"
#include "PingPongTexture.h"
#include "types.h"
#include "Address.h"
#include "Value.h"
#include "ValueStore.h"

namespace frag {
    class Module {
        public:
            struct Param {
                public:
                    std::variant<std::monostate, Address, Value> value;
                    std::variant<std::monostate, Address, Value> amp;
                    std::variant<std::monostate, Address, Value> shift;
            };

            Module(const std::string& output, const std::string& path, const Resolution& res);

            void setParam(const std::string& input, Param src);
            const std::string& getOutput() const;
            const std::string& getPath() const;
            std::shared_ptr<ShaderProgram> getShaderProgram();
            GLuint getFBO();
            GLenum getReadableBuf();
            std::shared_ptr<Texture> getLastOutTex();

            void setValues(std::shared_ptr<ValueStore> store);

            void bind();
            void unbind();

            void compile(std::shared_ptr<ValueStore> store);

        private:
            const std::string output_;
            const std::string path_;
            std::map<std::string, Param> params_;
            std::map<std::string, std::variant<std::monostate, Address, Value>> uniforms_;
            std::shared_ptr<PingPongTexture> ping_pong_;
            GLuint fbo_;
            const Resolution resolution_;
            std::shared_ptr<ShaderProgram> program_;

            static std::string toChannelName(const std::string& name);
    };
}

#endif
