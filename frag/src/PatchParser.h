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
#include "Media.h"
#include "Module.h"
#include "types.h"
#include "midi/Device.h"

namespace frag {
    class PatchParser {
        public:
            PatchParser(const std::string& path);

            std::map<std::string, std::shared_ptr<Media>> getMedia();
            std::vector<std::shared_ptr<Module>> getModules();
            std::map<std::string, std::shared_ptr<midi::Device>> getControllers();

            std::shared_ptr<ValueStore> getValueStore();

            Resolution getResolution() const;

        private:
            std::shared_ptr<Media> loadImage(const std::string& name, const YAML::Node& settings) const;
            std::shared_ptr<Media> loadVideo(const std::string& name, const YAML::Node& settings) const;
            std::shared_ptr<midi::Device> loadMidiDevice(const std::string& name, const YAML::Node& settings) const;

            const std::string path_;
            std::shared_ptr<ValueStore> store_;
    };
}

#endif
