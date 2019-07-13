#include "Address.h"


namespace frag {
    Address::Address(const std::string& name) : Address(name, "") {
    }

    Address::Address(const std::string& name, const std::string& field) : name_(name), field_(field) {
    }

    std::string Address::getField() const {
        return field_;
    }

    std::string Address::getName() const {
        return name_;
    }
    // This can often be seen written as
    bool Address::operator <(const Address& b) const {
        return (name_ < b.name_) ||
            ((name_ == b.name_) && (field_ < b.field_));
    }
}
