#include "Overwrite.h"

// STL
#include <stdexcept>

namespace frag {
    namespace cmd {
        void Overwrite::run(std::shared_ptr<ValueStore> store) const {
            Address src = std::get<Address>(args_.at(0));
            Address group_addr = src.withoutTail();
            const std::string tail = src.getTail();
            std::shared_ptr<Group> group = store->getGroup(group_addr);
            if (group == nullptr) {
                throw std::runtime_error(
                        "Command '" + name_ + "' expects argument 1 to be a member of a group." +
                        " Instead was '" + src.toString() + "'");
            }

            group->overwrite(tail, args_.at(1));

            store->set(group_addr, group);
        }

        void Overwrite::validate() const {
            if (args_.size() != 2) {
                throw std::runtime_error("Command '" + name_ + "' requires 2 arguments, arg 1: src, arg 2: dest.");
            }

            if (!isAddress(args_.at(0))) {
                throw std::runtime_error(
                        "Command '" + name_ + "' expected argument 1 to be an address");
            }
        }
    }
}
