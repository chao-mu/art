#include "Rotate.h"

// STL
#include <stdexcept>

// Ours
#include "../Group.h"

namespace frag {
    namespace cmd {
        void Rotate::run(std::shared_ptr<ValueStore> store) const {
            Address target = std::get<Address>(args_.at(0));
            std::shared_ptr<Group> group = store->getGroup(target);
            if (group == nullptr) {
                throw std::runtime_error("Command '" + name_ + "' expected '" +
                        target.toString() + "' to be a group.");
            }

            group->rotate();
        }

        void Rotate::validate() const {
            if (args_.size() != 1) {
                throw std::runtime_error("Command '" + name_ + "' expects 1 argument, the target");
            }

            if (!isAddress(args_.at(0))) {
                throw std::runtime_error("Command '" + name_ + "' expects its 1 argument to be a group address");
            }
        }
    }
}
