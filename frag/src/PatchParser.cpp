#include "PatchParser.h"

// STL
#include <stdexcept>
#include <optional>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Texture.h"
#include "Video.h"

#define KEY_MEDIAS "media"
#define KEY_GROUPS "groups"
#define KEY_MODULES "modules"
#define KEY_PLAYBACK "playback"
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
    PatchParser::PatchParser(const std::string& path) : path_(path), store_(std::make_shared<ValueStore>()) {}

    void PatchParser::parse() {
        parseMedia();
        parseControllers();
        parseModules();
        parseGroups();
    }

    std::shared_ptr<ValueStore> PatchParser::getValueStore() {
        return store_;
    }

    std::map<std::string, std::shared_ptr<Video>> PatchParser::getVideos() {
        return videos_;
    }

    std::map<std::string, std::shared_ptr<Texture>> PatchParser::getImages() {
        return images_;
    }

    std::map<std::string, std::shared_ptr<midi::Device>> PatchParser::getControllers() {
        return controllers_;
    }

    std::map<std::string, std::shared_ptr<Group>> PatchParser::getGroups() {
        return groups_;
    }

    void PatchParser::parseGroups() {
        const YAML::Node patch = YAML::LoadFile(path_);

        if (!patch[KEY_GROUPS]) {
            return;
        }

        for (const auto& kv : patch[KEY_GROUPS]) {
            const std::string& name = kv.first.as<std::string>();
            auto group = std::make_shared<Group>();

            for (const auto el : kv.second) {
                group->add(readAddressOrValue(el));
            }

            groups_[kv.first.as<std::string>()] = group;
            store_->set(Address(name), group);
        }
    }

    void PatchParser::parseControllers() {
        const YAML::Node patch = YAML::LoadFile(path_);

        if (!patch[KEY_CONTROLLERS]) {
            return;
        }

        for (const auto& kv : patch[KEY_CONTROLLERS]) {
            const std::string name = kv.first.as<std::string>();
            const YAML::Node& settings = kv.second;

            if (!settings[KEY_TYPE]) {
                throw std::runtime_error("controller '" + name + "' is missing type");
            }

            const std::string type = settings[KEY_TYPE].as<std::string>();
            if (type == CONTROLLER_TYPE_MIDI) {
                controllers_[name] = loadMidiDevice(name, settings);
            } else {
                throw std::runtime_error("unsupported controller type " + type);
            }
        }
    }

    void PatchParser::parseMedia() {
        const YAML::Node patch = YAML::LoadFile(path_);

        if (!patch[KEY_MEDIAS]) {
            return;
        }

        for (const auto& kv : patch[KEY_MEDIAS]) {
            const std::string name = kv.first.as<std::string>();
            const YAML::Node& settings = kv.second;

            store_->setIsMedia(Address(name), true);

            if (!settings[KEY_TYPE]) {
                throw std::runtime_error("source '" + name + "' is missing type");
            }

            const std::string type = settings[KEY_TYPE].as<std::string>();
            if (type == MEDIA_TYPE_IMAGE) {
                images_[name] = loadImage(name, settings);
            } else if (type == MEDIA_TYPE_VIDEO) {
                videos_[name] = loadVideo(name, settings);
            } else {
                throw std::runtime_error("unsupported media type " + type);
            }
        }
    }

    AddressOrValue PatchParser::readAddressOrValue(const YAML::Node& node) {
        if (node.IsSequence()) {
            std::vector<float> v = {};

            for (const auto& el : node) {
                v.push_back(el.as<float>());
            }

            return Value(v);
        }

        const std::string str = node.as<std::string>();

        bool b;
        float f;
        if (YAML::convert<bool>::decode(node, b) && str != "n" && str != "y") {
            return Value(b);
        }

        if (YAML::convert<float>::decode(node, f)) {
            return Value(f);
        }

        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(str);
        while (std::getline(iss, token, '.')) {
            tokens.push_back(token);
        }

        std::string swiz;
        if (tokens.size() > 1) {
            std::regex nonswiz_re("[^xyzwrgb]");

            if (!regex_search(tokens.back(), nonswiz_re)) {
                swiz = tokens.back();
            }
        }

        Address addr(tokens);
        addr.setSwiz(swiz);

        return addr;
    }

    std::vector<std::shared_ptr<Module>> PatchParser::getModules() {
        return modules_;
    }

    void PatchParser::parseModules() {
        const YAML::Node patch = YAML::LoadFile(path_);

        if (!patch[KEY_MODULES]) {
            return;
        }

        Resolution res = getResolution();

        for (const auto& settings : patch[KEY_MODULES]) {
            if (!settings[KEY_OUTPUT]) {
                throw std::runtime_error("A module is missing output");
            }

            const std::string output = settings[KEY_OUTPUT].as<std::string>();
            store_->setIsMedia(Address(output), true);

            if (!settings[KEY_PATH]) {
                throw std::runtime_error("A module is missing path");
            }

            const std::string path = settings[KEY_PATH].as<std::string>();

            auto mod = std::make_shared<Module>(output, path, res);

            if (settings[KEY_INPUTS]) {
                for (const auto& input_kv : settings[KEY_INPUTS]) {
                    const std::string key = input_kv.first.as<std::string>();
                    const YAML::Node& value = input_kv.second;

                    Module::Param param;

                    if (value.Type() == YAML::NodeType::Map) {
                        param.value = readAddressOrValue(value[KEY_INPUT]);

                        if (value[KEY_AMP]) {
                            param.amp = readAddressOrValue(value[KEY_AMP]);
                        }

                        if (value[KEY_SHIFT]) {
                            param.shift = readAddressOrValue(value[KEY_SHIFT]);
                        }
                    } else {
                        param.value = readAddressOrValue(value);
                    }

                    mod->setParam(key, param);
                }
            }

            int repeat = 1;
            if (settings[KEY_REPEAT]) {
                repeat = settings[KEY_REPEAT].as<int>();
            }

            for (int i = 0; i < repeat; i++) {
                modules_.push_back(mod);
            }
        }
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

    std::shared_ptr<Video> PatchParser::loadVideo(const std::string& name, const YAML::Node& settings) const {
        if (!settings[KEY_PATH]) {
            throw std::runtime_error("media '" + name + "' is missing path");
        }

        const std::string path = settings[KEY_PATH].as<std::string>();

        auto vid = std::make_shared<Video>(path);
        if (settings[KEY_SCALE_FILTER]) {
            const std::string filter = settings[KEY_SCALE_FILTER].as<std::string>();
            if (filter == "nearest") {
                vid->setScaleFilter(GL_NEAREST, GL_NEAREST);
            } else {
                throw std::runtime_error("Invalid scale filter");
            }
        }

        if (settings[KEY_PLAYBACK]) {
            const std::string playback = settings[KEY_PLAYBACK].as<std::string>();
            if (playback == "reverse") {
                vid->setReverse(true);
            }
        }

        vid->start();

        return vid;
    }

    std::shared_ptr<Texture> PatchParser::loadImage(const std::string& name, const YAML::Node& settings) const {
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
