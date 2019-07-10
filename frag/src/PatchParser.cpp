#include "PatchParser.h"

// STL
#include <stdexcept>
#include <optional>
#include <regex>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Texture.h"
#include "Camera.h"

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
#define KEY_SCALE_FILTER "sizeFilter"
#define KEY_CONTROLLERS "controllers"
#define CONTROLLER_TYPE_MIDI "midi"
#define MEDIA_TYPE_IMAGE "image"
#define MEDIA_TYPE_VIDEO "video"

namespace frag {
    PatchParser::PatchParser(const std::string& path) : path_(path) {}

    std::map<std::string, std::shared_ptr<midi::Device>> PatchParser::getControllers() const {
        const YAML::Node patch = YAML::LoadFile(path_);
        std::map<std::string, std::shared_ptr<midi::Device>> controllers;

        if (!patch[KEY_CONTROLLERS]) {
            return controllers;
        }

        for (const auto& kv : patch[KEY_CONTROLLERS]) {
            const std::string name = kv.first.as<std::string>();
            const YAML::Node& settings = kv.second;

            if (!settings[KEY_TYPE]) {
                throw std::runtime_error("controller '" + name + "' is missing type");
            }

            const std::string type = settings[KEY_TYPE].as<std::string>();
            if (type == CONTROLLER_TYPE_MIDI) {
                controllers[name] = loadMidiDevice(name, settings);
            } else {
                throw std::runtime_error("unsupported controller type " + type);
            }
        }

        return controllers;
    }

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
            } else if (type == MEDIA_TYPE_VIDEO) {
                media[name] = loadVideo(name, settings);
            } else {
                throw std::runtime_error("unsupported media type " + type);
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
                const std::regex tex_re(R"(^(\w+)(?:\.([rgbaxyzw]{1,4}))?)");
                const std::regex ctrl_re(R"(^(\w+)\.(\w+))");

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
                        std::smatch match;
                        if (std::regex_match(svalue, match, tex_re)) {
                            src.tex_name = match[1];

                            const std::string swiz = match[2];
                            if (swiz.size() > 0) src.first = swiz[0];
                            if (swiz.size() > 1) src.second = swiz[1];
                            if (swiz.size() > 2) src.third = swiz[2];
                            if (swiz.size() > 3) src.fourth = swiz[3];
                        } else if (std::regex_match(svalue, match, ctrl_re)) {
                            src.controller = match[1];
                            src.control = match[2];
                        } else {
                            throw std::runtime_error("Invalid source specified for input " + key);
                        }
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

                    auto program = mod->getShaderProgram();
                    const std::string uni_name = Module::toChannelName(param_name);

                    bool b;
                    float f;
                    if (YAML::convert<bool>::decode(param_value, b)) {
                        program->setUniform(uni_name, b);
                    } else if (YAML::convert<float>::decode(param_value, f)) {
                        program->setUniform(uni_name, f);
                    } else if (param_value.IsSequence()) {
                        std::vector<float> v = {};

                        for (const auto& el : param_value) {
                            v.push_back(el.as<float>());
                        }

                        program->setUniform(uni_name, v);
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

    std::shared_ptr<midi::Device> PatchParser::loadMidiDevice(const std::string& name, const YAML::Node& settings) const {
        if (!settings[KEY_PATH]) {
            throw std::runtime_error("controller '" + name + "' is missing path");
        }

        const std::string path = settings[KEY_PATH].as<std::string>();

        auto dev = std::make_shared<midi::Device>(path);
        dev->start();

        return dev;
    }

    std::shared_ptr<Media> PatchParser::loadVideo(const std::string& name, const YAML::Node& settings) const {
        if (!settings[KEY_PATH]) {
            throw std::runtime_error("media '" + name + "' is missing path");
        }

        const std::string path = settings[KEY_PATH].as<std::string>();

        auto vid = std::make_shared<Camera>(path);
        if (settings[KEY_SCALE_FILTER]) {
            const std::string filter = settings[KEY_SCALE_FILTER].as<std::string>();
            if (filter == "nearest") {
                vid->setScaleFilter(GL_NEAREST, GL_NEAREST);
            } else {
                throw std::runtime_error("Invalid scale filter");
            }
        }

        vid->start();
        vid->update();

        return vid;
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
