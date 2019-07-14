#ifndef FRAG_MEDIA_H_
#define FRAG_MEDIA_H_

// STL
#include <string>

// Ours
#include "types.h"

namespace frag {
    class Media {
        public:
            virtual void update();
            virtual void bind(unsigned int slot) = 0;
            virtual void unbind() = 0;
            virtual void play();
            virtual void pause();
            virtual Resolution getResolution() = 0;
    };
}
#endif
