#include "AddressOrValue.h"

namespace frag {
    bool isAddress(const AddressOrValue& aov) {
        return std::holds_alternative<Address>(aov);
    }

    bool isValue(const AddressOrValue& aov) {
        return std::holds_alternative<Value>(aov);
    }
}
