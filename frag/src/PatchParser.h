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
#include "cmd/Command.h"

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
            std::vector<std::shared_ptr<cmd::Command>> getCommands();

            Resolution getResolution() const;

        private:
            AddressOrValue readAddressOrValue(const YAML::Node& node);
            Address readAddress(const YAML::Node& node);
            const YAML::Node requireNode(const YAML::Node& parent, const std::string& key, const std::string& err);
            Address requireAddress(const YAML::Node& parent, const std::string& key, const std::string& err);

            void parseMedia(const YAML::Node& patch);
            void parseCommands(const YAML::Node& patch);
            void parseControllers(const YAML::Node& patch);
            void parseModules(const YAML::Node& patch);
            void parseGroups(const YAML::Node& patch);

            std::shared_ptr<Texture> loadImage(const std::string& name, const std::string& path, const YAML::Node& settings) const;
            std::shared_ptr<Video> loadVideo(const std::string& name, const std::string& path, const YAML::Node& settings) const;
            std::shared_ptr<midi::Device> loadMidiDevice(const std::string& name, const YAML::Node& settings) const;
            std::shared_ptr<cmd::Command> loadCommand(int num, const std::string& name, Address trigger, std::vector<AddressOrValue> args);

            const std::string path_;
            std::shared_ptr<ValueStore> store_;
            std::map<std::string, std::shared_ptr<Video>> videos_;
            std::map<std::string, std::shared_ptr<Texture>> images_;
            std::map<std::string, std::shared_ptr<midi::Device>> controllers_;
            std::vector<std::shared_ptr<Module>> modules_;
            std::map<std::string, std::shared_ptr<Group>> groups_;
            std::vector<std::shared_ptr<cmd::Command>> commands_;
    };
}

#endif
