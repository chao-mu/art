#ifndef FRAG_MEDIA_H_
#define FRAG_MEDIA_H_

// STL
#include <string>

namespace frag {
    class Media {
        public:
            virtual void update();
            virtual void bind(unsigned int slot) = 0;
            virtual void unbind() = 0;
            virtual void play();
            virtual void pause();
    };
}
#endif
