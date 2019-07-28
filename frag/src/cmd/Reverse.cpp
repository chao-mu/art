#include "Reverse.h"

// STL
#include <stdexcept>

// Ours
#include "../Video.h"

namespace frag {
    namespace cmd {
        void Reverse::run(std::shared_ptr<ValueStore> store) const {
            Address target = std::get<Address>(args_.at(0));
            std::shared_ptr<Video> video = store->getVideo(target);
            if (video == nullptr) {
                throw std::runtime_error("Command '" + name_ + "' expected '" +
                        target.toString() + "' to be a video.");
            }

            video->flipPlayback();
        }

        void Reverse::validate() const {
            if (args_.size() != 1) {
                throw std::runtime_error("Command '" + name_ + "' expects 1 argument, the target");
            }

            if (!isAddress(args_.at(0))) {
                throw std::runtime_error("Command '" + name_ + "' expects its 1 argument to be a video address");
            }
        }
    }
}
