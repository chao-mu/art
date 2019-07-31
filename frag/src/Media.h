#ifndef FRAG_MEDIA_H_
#define FRAG_MEDIA_H_

// STL
#include <string>

// Ours
#include "Resolution.h"

namespace frag {
    class Media {
        public:
            virtual void update();
            virtual void bind(unsigned int slot) = 0;
            virtual void unbind() = 0;
            virtual Resolution getResolution() = 0;
            virtual void inFocus();
            virtual void outFocus();

            bool isInUse();
            bool wasInUse();
            void resetInUse();

        private:
            bool in_use_ = false;
            bool last_in_use_ = false;
    };
}
#endif
