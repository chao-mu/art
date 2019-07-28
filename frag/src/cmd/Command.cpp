#include "Command.h"

#include <stdexcept>

namespace frag {
    namespace cmd {
        Command::Command(const std::string& name, const Address& trigger, std::vector<AddressOrValue> args) :
            name_(name), trigger_(trigger), args_(args) {}

        void Command::throwIncompatible() const {
            throw std::runtime_error(
                    "Command '" + name_ + "' is incompatible with target '" + target_.toString() + "'");
        }

        Address Command::getTrigger() const {
            return trigger_;
        }
    }

}

