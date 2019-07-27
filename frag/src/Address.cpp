#include "Address.h"

namespace frag {
    Address::Address(std::vector<std::string> fields) : fields_(fields) {}

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

    std::string Address::getSwiz() {
        return swiz_;
    }

    void Address::setSwiz(const std::string& str) {
        swiz_ = str;
    }

    std::string Address::getHead() {
        return fields_.front();
    }

    std::vector<std::string> Address::getFields() const {
        return fields_;
    }

    Address Address::withoutTail() const {
        return Address(std::vector(fields_.cbegin(), fields_.cend() - 1));
    }

    Address Address::operator+(const std::string& str) const {
        std::vector<std::string> copy = fields_;
        copy.push_back(str);
        return Address(copy);
    }
    // This can often be seen written as
    bool Address::operator<(const Address& b) const {
        return toString() < b.toString();
    }
}
