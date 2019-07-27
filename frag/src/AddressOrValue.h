#ifndef ADDRESS_OR_VALUE_H_
#define ADDRESS_OR_VALUE_H_

// STL
#include <string>
#include <variant>

// Ours
#include "Address.h"
#include "Value.h"

namespace frag {
    using AddressOrValue = std::variant<std::monostate, Address, Value>;
    bool isAddress(const AddressOrValue& aov);
    bool isValue(const AddressOrValue& aov);
}

#endif
