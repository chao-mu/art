#include "Address.h"

namespace frag {
    Address::Address(const std::vector<std::string>& fields) : fields_(fields) {}
    Address::Address(const std::vector<std::string>& fields, const std::string& tail) : fields_(fields) {
        fields_.push_back(tail);
    }

    std::string Address::toString() const {
        std::string str;

        std::string sep = "";
        for (const auto& field : fields_) {
            // TODO: Remove PatchParser call to our constructor with empty values. Boo
            if (field.empty()) {
                continue;
            }

            str += sep;
            str += field;
            sep = ".";
        }

        return str;
    }

    Address Address::withoutTail() const {
        return Address(std::vector(fields_.cbegin(), fields_.cend() - 1));
    }


    std::string Address::getSwiz() {
        return swiz_;
    }

    void Address::setSwiz(const std::string& str) {
        swiz_ = str;
    }

    std::string Address::getTail() {
        return fields_.back();
    }

    std::string Address::getHead() {
        return fields_.front();
    }

    std::vector<std::string> Address::getFields() const {
        return fields_;
    }

    Address Address::operator+(const std::string& str) const {
        return Address(fields_, str);
    }
    // This can often be seen written as
    bool Address::operator<(const Address& b) const {
        return toString() < b.toString();
    }
}
