#ifndef FRAG_ADDRESS_H_
#define FRAG_ADDRESS_H_

// STL
#include <string>

namespace frag {
    class Address {
        public:
            Address(const std::string& name);
            Address(const std::string& name, const std::string& field);
            Address(const std::string& name, const std::string& field, const std::string& sub_field);

            std::string getField() const;
            std::string getSubField() const;
            std::string getName() const;

            Address withSubField(const std::string& sub) const;

            bool operator <(const Address& b) const;
            Address operator +(const std::string& str) const;
        private:
            std::string name_;
            std::string field_;
            std::string sub_field_;
    };
}
#endif
