#include "Group.h"

#include <algorithm>

namespace frag {
    void Group::add(AddressOrValue aov) {
        elements_.push_back(aov);

        if (elements_.size() == 1)  {
            setMapping("first", 0);
        } else if (elements_.size() == 2)  {
            setMapping("second", 1);
        }
    }

    void Group::rotate() {
        std::rotate(elements_.begin(), elements_.begin(), elements_.end());
    }

    AddressOrValue Group::exchange(const std::string& key, AddressOrValue aov) {
        AddressOrValue old = elements_.at(mappings_.at(key));
        elements_[mappings_.at(key)] = aov;

        return old;
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
