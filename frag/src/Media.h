#ifndef FRAG_MEDIA_H_
#define FRAG_MEDIA_H_

// STL
#include <string>

// Ours
#include "types.h"

namespace frag {
    // TODO Reduce the usage of this so only applicable methods are here defined.
    //      or eliminate usage entirely.
    class Media {
        public:
            virtual void update();
            virtual void bind(unsigned int slot) = 0;
            virtual void unbind() = 0;
            virtual void play();
            virtual void pause();
            virtual void flipPlayback();
            virtual Resolution getResolution() = 0;
    };
}
#endif
