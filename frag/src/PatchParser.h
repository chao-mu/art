#ifndef FRAG_PATCHPARSER_H_
#define FRAG_PATCHPARSER_H_

// STL
#include <map>
#include <optional>
#include <utility>
#include <variant>

// yaml-cpp
#include "yaml-cpp/yaml.h"

// Ours
#include "Video.h"
#include "Texture.h"
#include "Module.h"
#include "Resolution.h"
#include "AddressOrValue.h"
#include "midi/Device.h"

namespace frag {
    class PatchParser {
        public:
            PatchParser(const std::string& path);

            void parse();

            std::map<std::string, std::shared_ptr<Video>> getVideos();
            std::map<std::string, std::shared_ptr<Texture>> getImages();
            std::vector<std::shared_ptr<Module>> getModules();
            std::map<std::string, std::shared_ptr<midi::Device>> getControllers();
            std::map<std::string, std::shared_ptr<Group>> getGroups();
            std::shared_ptr<ValueStore> getValueStore();

            Resolution getResolution() const;

        private:
            static AddressOrValue readAddressOrValue(const YAML::Node& node);

            void parseMedia();
            void parseControllers();
            void parseModules();
            void parseGroups();

            std::shared_ptr<Texture> loadImage(const std::string& name, const YAML::Node& settings) const;
            std::shared_ptr<Video> loadVideo(const std::string& name, const YAML::Node& settings) const;
            std::shared_ptr<midi::Device> loadMidiDevice(const std::string& name, const YAML::Node& settings) const;

            const std::string path_;
            std::shared_ptr<ValueStore> store_;
            std::map<std::string, std::shared_ptr<Video>> videos_;
            std::map<std::string, std::shared_ptr<Texture>> images_;
            std::map<std::string, std::shared_ptr<midi::Device>> controllers_;
            std::vector<std::shared_ptr<Module>> modules_;
            std::map<std::string, std::shared_ptr<Group>> groups_;
    };
}

#endif
