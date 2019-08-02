#ifndef FRAG_IMAGE_H_
#define FRAG_IMAGE_H_

// STL
#include <string>

namespace frag {
    class Image {
        public:
            static bool isImage(const std::string& path);
    };
}

#endif
