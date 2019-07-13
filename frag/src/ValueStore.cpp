#include "ValueStore.h"

#include "MathUtil.h"

namespace frag {
    bool ValueStore::isMedia(const Address& addr) const {
        const std::string name = addr.getName();

        return is_media_.count(name) > 0 && is_media_.at(name);
    }

    void ValueStore::setIsMedia(const Address& addr, bool is_media) {
        is_media_[addr.getName()] = is_media;
    }

    std::optional<Value> ValueStore::getValue(const Address& addr) const {
        if (values_.count(addr)) {
            return values_.at(addr);
        } else {
            return {};
        }
    }

    std::shared_ptr<Media> ValueStore::getMedia(const Address& addr) const {
        if (media_.count(addr.getName())) {
            return media_.at(addr.getName());
        } else {
            return nullptr;
        }
    }

    void ValueStore::set(Address addr, Value v) {
        values_[addr] = v;
    }

    void ValueStore::set(Address addr, std::shared_ptr<Media> m) {
        setIsMedia(addr, true);
        media_[addr.getName()] = m;
    }

    void ValueStore::set(Address addr, midi::Control c) {
        if (c.type == midi::CONTROL_TYPE_BUTTON) {
            values_[addr] = Value(c.pressed);
        } else {
            values_[addr] = Value(remap(c.value, c.low, c.high, 0, 1));
        }
    }
}
