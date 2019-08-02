#ifndef FRAG_FILEUTIL_H_
#define FRAG_FILEUTIL_H_

#include <string>

namespace frag {
    namespace fileutil {
        std::string slurp(const std::string& path);
        std::string slurp(const std::string& relative_to, const std::string& path);
        bool hasExtension(const std::string& path, const std::string& ext);
    }
}
#endif
