#include "Group.h"

namespace frag {
    void Group::add(AddressOrValue aov) {
        elements_.push_back(aov);

        if (elements_.size() == 1)  {
            setMapping("first", 0);
        }
    }

    void Group::overwrite(const std::string& key, AddressOrValue aov) {
        elements_[mappings_.at(key)] = aov;
    }

    std::map<std::string, AddressOrValue> Group::getMappings() {
        std::map<std::string, AddressOrValue> mappings;

        for (const auto& kv : mappings_) {
            mappings[kv.first] = elements_.at(kv.second);
        }

        return mappings;
    }

    void Group::setMapping(const std::string& key, int i) {
        mappings_[key] = i;
    }
}
