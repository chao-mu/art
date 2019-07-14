#include "Control.h"

namespace frag {
    namespace midi {
        bool Control::isPressed() {
            return value > 0.5;
        }
    }
}
