#include "ValueStore.h"

#include "MathUtil.h"

#include <iostream>

namespace frag {
    bool ValueStore::isMedia(Address addr) const {
        addr = getAddress(addr);

        return is_media_.count(addr) > 0 && is_media_.at(addr);
    }

    Address ValueStore::getAddress(Address addr) const {
        if (values_.count(addr) > 0 || is_media_.count(addr) > 0 || groups_.count(addr) > 0) {
            return addr;
        }

        std::vector<std::string> fields = addr.getFields();
        std::vector<std::string> tail;
        while (!fields.empty()) {
            Address attempt = Address(fields);
            if (aovs_.count(attempt)) {
                const AddressOrValue& aov = aovs_.at(attempt);
                if (isAddress(aov)) {
                    std::vector<std::string> aov_fields = std::get<Address>(aov).getFields();
                    tail.insert(tail.begin(), aov_fields.begin(), aov_fields.end());
                    return Address(tail);
                } else {
                    return addr;
                }
            }

            tail.push_back(fields.back());
            fields.pop_back();
        }

        return addr;
    }

    void ValueStore::setIsMedia(Address addr, bool is_media) {
        is_media_[addr] = is_media;
    }

    std::optional<Value> ValueStore::getValue(Address addr) const {
        addr = getAddress(addr);
        if (values_.count(addr)) {
            return values_.at(addr);
        } else if (aovs_.count(addr)) {
            AddressOrValue aov = aovs_.at(addr);
            if (isValue(aov)) {
                return std::get<Value>(aov);
            }
        }

        return {};
    }

    std::shared_ptr<Video> ValueStore::getVideo(Address addr) const {
        addr = getAddress(addr);

        if (videos_.count(addr)) {
            return videos_.at(addr);
        } else {
            return nullptr;
        }
    }

    std::shared_ptr<Group> ValueStore::getGroup(Address addr) const {
        addr = getAddress(addr);

        if (groups_.count(addr)) {
            return groups_.at(addr);
        } else {
            return nullptr;
        }
    }

    std::shared_ptr<Media> ValueStore::getMedia(Address addr) const {
        addr = getAddress(addr);

        if (media_.count(addr)) {
            return media_.at(addr);
        } else {
            return nullptr;
        }
    }

    void ValueStore::set(Address addr, std::shared_ptr<Group> g) {
        for (const auto& kv : g->getMappings()) {
            aovs_[addr + kv.first] = kv.second;
        }

        groups_[addr] = g;
    }

    void ValueStore::set(Address addr, Value v) {
        values_[addr] = v;
    }

    void ValueStore::set(Address addr, std::shared_ptr<Video> v) {
        videos_[addr] = v;
        setMedia(addr, v);
    }

    void ValueStore::set(Address addr, std::shared_ptr<Texture> t) {
        setMedia(addr, t);
    }

    void ValueStore::setMedia(Address addr, std::shared_ptr<Media> m) {
        setIsMedia(addr, true);
        media_[addr] = m;
        Resolution res = m->getResolution();
        set(addr + "resolution", frag::Value(std::vector({static_cast<float>(res.width), static_cast<float>(res.height)})));
    }

    void ValueStore::set(Address addr, midi::Control c) {
        if (c.type == midi::CONTROL_TYPE_BUTTON) {
            std::optional<Value> last_opt = getValue(addr + "hold");
            bool last_pressed = last_opt.has_value() && last_opt.value().getBool();

            values_[addr + "hold"] = Value(c.isPressed());

            values_[addr + "release"] = Value(last_pressed && !c.isPressed());
            values_[addr + "press"] = Value(!last_pressed && c.isPressed());
            values_[addr] = Value(static_cast<float>(c.value) >= 0.5 ? true : false);
        } else {
            values_[addr] = Value(remap(c.value, c.low, c.high, 0, 1));
        }
    }
}
