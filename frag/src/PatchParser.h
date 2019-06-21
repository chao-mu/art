#ifndef FRAG_PATCHPARSER_H_
#define FRAG_PATCHPARSER_H_

// STL
#include <map>
#include <optional>
#include <utility>

// yaml-cpp
#include "yaml-cpp/yaml.h"

// Ours
#include "Media.h"
#include "Module.h"
#include "types.h"

namespace frag {
    class PatchParser {
        public:
            PatchParser(const std::string& path);

            std::map<std::string, std::shared_ptr<Media>> getMedia() const;
            std::vector<std::shared_ptr<Module>> getModules() const;
            Resolution getResolution() const;

        private:
            std::shared_ptr<Media> loadImage(const std::string& name, const YAML::Node& settings) const;
            const std::string path_;
    };
}

#endif
