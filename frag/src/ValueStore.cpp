#include "ValueStore.h"

#include "MathUtil.h"

#include <iostream>

namespace frag {
    bool ValueStore::isMedia(Address addr) const {
        addr = getAddress(addr);

        return is_media_.count(addr) > 0 && is_media_.at(addr);
    }

    Address ValueStore::getAddress(Address addr) const {
        if (values_.count(addr) > 0 || is_media_.count(addr) > 0) {
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

    std::shared_ptr<Media> ValueStore::getMedia(Address addr) const {
        addr = getAddress(addr);

        if (media_.count(addr)) {
            return media_.at(addr);
        } else {
            return nullptr;
        }
    }

    void ValueStore::set(Address addr, std::shared_ptr<Group> g) {
        aovs_[addr + "first"] = g->first();
    }

    void ValueStore::set(Address addr, Value v) {
        values_[addr] = v;
    }

    void ValueStore::set(Address addr, std::shared_ptr<Media> m) {
        setIsMedia(addr, true);
        media_[addr] = m;
        Resolution res = m->getResolution();
        set(addr + "resolution", frag::Value(std::vector({static_cast<float>(res.width), static_cast<float>(res.height)})));
    }

    void ValueStore::set(Address addr, midi::Control c) {
        if (c.type == midi::CONTROL_TYPE_BUTTON) {
            values_[addr] = Value(c.isPressed());
            values_[addr + "toggle"] = Value(c.toggle);
        } else {
            values_[addr] = Value(remap(c.value, c.low, c.high, 0, 1));
        }
    }
}
