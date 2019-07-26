#include "Address.h"


namespace frag {
    Address::Address(const std::string& name) : Address(name, "") {}

    Address::Address(const std::string& name, const std::string& field) : Address(name, field, "") {}

    Address::Address(const std::string& name, const std::string& field, const std::string& sub_field) :
        name_(name), field_(field), sub_field_(sub_field) {}


    std::string Address::getField() const {
        return field_;
    }

    std::string Address::getSubField() const {
        return sub_field_;
    }

    std::string Address::getName() const {
        return name_;
    }

    Address Address::withSubField(const std::string& sub) const {
        return Address(getName(), getField(), sub);
    }

    Address Address::operator+(const std::string& str) const {
        if (field_ == "") {
            return Address(name_, str);
        } else {
            return withSubField(str);
        }
    }
    // This can often be seen written as
    bool Address::operator<(const Address& b) const {
        // HACK, if addresses have !!! in them we're messed up
        return (name_ + "!!!" + field_ + "!!!" + sub_field_) <
            (b.name_ + "!!!" + b.field_ + "!!!" + b.sub_field_);
    }
}
