#ifndef FRAG_MIDI_CONTROL_H_
#define FRAG_MIDI_CONTROL_H_

// STL
#include <string>

namespace frag {
    namespace midi {
        enum ControlType {
            CONTROL_TYPE_BUTTON,
            CONTROL_TYPE_FADER,
            CONTROL_TYPE_UNKNOWN
        };

        struct Control {
            std::string name = "";
            ControlType type = CONTROL_TYPE_UNKNOWN;
            bool pressed = false;
            unsigned char value = 0;
            unsigned char low = 0;
            unsigned char high = 0;
            unsigned char function = 0;
            unsigned char channel = 0;
        };
    }
}
#endif
