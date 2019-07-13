#ifndef FRAG_ADDRESS_H_
#define FRAG_ADDRESS_H_

// STL
#include <string>

namespace frag {
    class Address {
        public:
            Address(const std::string& name);
            Address(const std::string& name, const std::string& field);

            std::string getField() const;
            std::string getName() const;
            bool operator <(const Address& b) const;

        private:
            std::string name_;
            std::string field_;
    };
}
#endif
