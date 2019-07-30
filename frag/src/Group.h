#ifndef FRAG_GROUP_H_
#define FRAG_GROUP_H_

// STL
#include <map>

// Ours
#include "AddressOrValue.h"

namespace frag {
    class Group {
        public:
            std::map<std::string, AddressOrValue> getMappings();
            void setMapping(const std::string& key, int i);

            void rotate();
            AddressOrValue exchange(const std::string& key, AddressOrValue aov);
            void overwrite(const std::string& key, AddressOrValue aov);
            void add(AddressOrValue aov);

        private:
            std::vector<AddressOrValue> elements_;
            std::map<std::string, int> mappings_;
    };
}

#endif
