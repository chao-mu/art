#include "PatchParser.h"

// STL
#include <stdexcept>
#include <optional>
#include <regex>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Texture.h"

#define KEY_MEDIAS "media"
#define KEY_MODULES "modules"
#define KEY_TYPE "type"
#define KEY_OUTPUT "output"
#define KEY_INPUTS "inputs"
#define KEY_PATH "path"
#define KEY_RESOLUTION "resolution"
#define KEY_WIDTH "width"
#define KEY_HEIGHT "height"
#define KEY_REPEAT "repeat"
#define KEY_INPUT "input"
#define KEY_AMP "amp"
#define KEY_SHIFT "shift"
#define MEDIA_TYPE_IMAGE "image"

namespace frag {
    PatchParser::PatchParser(const std::string& path) : path_(path) {}

    std::map<std::string, std::shared_ptr<Media>> PatchParser::getMedia() const {
        const YAML::Node patch = YAML::LoadFile(path_);
        std::map<std::string, std::shared_ptr<Media>> media;

        if (!patch[KEY_MEDIAS]) {
            return media;
        }

        for (const auto& kv : patch[KEY_MEDIAS]) {
            const std::string name = kv.first.as<std::string>();
            const YAML::Node& settings = kv.second;

            if (!settings[KEY_TYPE]) {
                throw std::runtime_error("source '" + name + "' is missing type");
            }

            const std::string type = settings[KEY_TYPE].as<std::string>();
            if (type == MEDIA_TYPE_IMAGE) {
                media[name] = loadImage(name, settings);
            }
        }

        return media;
    }

    std::vector<std::shared_ptr<Module>> PatchParser::getModules() const {
        const YAML::Node patch = YAML::LoadFile(path_);
        std::vector<std::shared_ptr<Module>> modules;

        if (!patch[KEY_MODULES]) {
            return modules;
        }

        Resolution res = getResolution();

        for (const auto& settings : patch[KEY_MODULES]) {
            if (!settings[KEY_OUTPUT]) {
                throw std::runtime_error("A module is missing output");
            }

            const std::string output = settings[KEY_OUTPUT].as<std::string>();

            if (!settings[KEY_PATH]) {
                throw std::runtime_error("A module is missing path");
            }

            const std::string path = settings[KEY_PATH].as<std::string>();

            auto mod = std::make_shared<Module>(output, path, res);

            if (settings[KEY_INPUTS]) {
                const std::regex numeric_re(R"(^-?\d+(?:\.\d+)?)");
                const std::regex src_re(R"(^(\w+)(?:\.([rgbaxyzw]{1,4}))?)");

                for (const auto& input_kv : settings[KEY_INPUTS]) {
                    const std::string key = input_kv.first.as<std::string>();
                    const YAML::Node& value = input_kv.second;

                    Module::Source src;

                    std::string svalue;
                    if (value.Type() == YAML::NodeType::Map) {
                        svalue = value[KEY_INPUT].as<std::string>();

                        if (value[KEY_AMP]) {
                            src.amp = value[KEY_AMP].as<float>();
                        }

                        if (value[KEY_SHIFT]) {
                            src.shift = value[KEY_SHIFT].as<float>();
                        }
                    } else if (value.Type() != YAML::NodeType::Sequence) {
                        svalue = value.as<std::string>();
                    }

                    bool test_bool;
                    float test_float;
                    if (svalue != "" &&
                            !YAML::convert<bool>::decode(value, test_bool) &&
                            !YAML::convert<float>::decode(value, test_float)) {
                        src.is_texture = true;

                        std::smatch src_match;
                        if (!std::regex_match(svalue, src_match, src_re)) {
                            throw std::runtime_error("Invalid source specified for input " + key);
                        }

                        src.name = src_match[1];

                        const std::string swiz = src_match[2];
                        if (swiz.size() > 0) src.first = swiz[0];
                        if (swiz.size() > 1) src.second = swiz[1];
                        if (swiz.size() > 2) src.third = swiz[2];
                        if (swiz.size() > 3) src.fourth = swiz[3];
                    } else {
                        src.is_texture = false;
                    }

                    mod->addSource(key, src);
                }
            }

            mod->compile();

            if (settings[KEY_INPUTS]) {
                mod->bind();
                for (const auto& kv : settings[KEY_INPUTS]) {
                    const std::string param_name = kv.first.as<std::string>();
                    const YAML::Node& param_value = kv.second;

                    std::shared_ptr<ShaderProgram> program = mod->getShaderProgram();
                    std::optional<GLenum> gl_type = program->getUniformType(
                            Module::toChannelName(param_name));
                    if (!gl_type.has_value()) {
                        throw std::runtime_error(
                                "Module entry with output '" +
                                output + "' and path '" + path + "' specifies non-existent uniform '" +
                                param_name);
                    }

                    const std::string uni_name = Module::toChannelName(param_name);

                    switch (gl_type.value()) {
                        case GL_FLOAT: {
                            float v = param_value.as<float>();
                            program->setUniform(uni_name, [&v](GLint& id) {
                                glUniform1f(id, v);
                            });
                            break;
                        }
                        case GL_INT: {
                            int v = param_value.as<int>();
                            program->setUniform(uni_name, [&v](GLint& id) {
                                glUniform1i(id, v);
                            });
                            break;
                        }
                        case GL_BOOL: {
                            bool v = param_value.as<bool>();
                            program->setUniform(uni_name, [&v](GLint& id) {
                                glUniform1i(id, v ? 1 : 0);
                            });
                            break;
                        }
                        case GL_FLOAT_VEC2: {
                            float v[2] = {0, 0};
                            int i = 0;
                            for (const auto& node : param_value) {
                                v[i++] = node.as<float>();
                            }

                            program->setUniform(uni_name, [&v](GLint& id) {
                                glUniform2f(id, v[0], v[1]);
                            });

                            break;
                        }
                        case GL_FLOAT_VEC3: {
                            float v[3] = {0, 0, 0};
                            int i = 0;
                            for (const auto& node : param_value) {
                                v[i++] = node.as<float>();
                            }

                            program->setUniform(uni_name, [&v](GLint& id) {
                                glUniform3f(id, v[0], v[1], v[2]);
                            });

                            break;
                        }
                        case GL_FLOAT_VEC4: {
                            float v[4] = {0, 0, 0, 0};
                            int i = 0;
                            for (const auto& node : param_value) {
                                v[i++] = node.as<float>();
                            }

                            program->setUniform(uni_name, [&v](GLint& id) {
                                glUniform4f(id, v[0], v[1], v[2], v[3]);
                            });

                            break;
                        }
                        case GL_SAMPLER_2D: break;
                        default:
                            throw std::runtime_error(
                                "Module with path " + path + " specifies unsupported uniform " + param_name);
                    }
                }
                mod->unbind();
            }

            int repeat = 1;
            if (settings[KEY_REPEAT]) {
                repeat = settings[KEY_REPEAT].as<int>();
            }

            for (int i = 0; i < repeat; i++) {
                modules.push_back(mod);
            }
        }

        return modules;
    }

    Resolution PatchParser::getResolution() const {
        const YAML::Node patch = YAML::LoadFile(path_);
        Resolution res;

        if (!patch[KEY_RESOLUTION]) {
            res.width = 1280;
            res.height = 960;
            return res;
        }

        const YAML::Node& res_node = patch[KEY_RESOLUTION];

        if (!res_node[KEY_WIDTH] || !res_node[KEY_HEIGHT]) {
            throw std::runtime_error("resolution section missing width or height");
        }

        res.width = res_node[KEY_WIDTH].as<int>();
        res.height = res_node[KEY_HEIGHT].as<int>();

        return res;
    }

    std::shared_ptr<Media> PatchParser::loadImage(const std::string& name, const YAML::Node& settings) const {
        if (!settings[KEY_PATH]) {
            throw std::runtime_error("source '" + name + "' is missing path");
        }

        const std::string img_path = settings[KEY_PATH].as<std::string>();

        cv::Mat image = cv::imread(img_path);
        if (image.empty()) {
            throw std::runtime_error("unable to load image " + img_path);
        }

        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        flip(image, image, 0);

        // Load image into texture
        auto image_tex = std::make_shared<frag::Texture>();
        image_tex->populate(image);

        return image_tex;
    }
}
