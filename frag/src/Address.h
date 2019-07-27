#ifndef FRAG_ADDRESS_H_
#define FRAG_ADDRESS_H_

// STL
#include <string>
#include <vector>

namespace frag {
    class Address {
        public:
            Address(std::vector<std::string> fields);
            template<class ...Ts> Address(Ts... fields) : fields_{fields...} {}

            std::string getHead();

            void setSwiz(const std::string& str);
            std::string getSwiz();

            Address withoutTail() const;

            bool operator <(const Address& b) const;
            Address operator +(const std::string& str) const;

            std::string toString() const;

            std::vector<std::string> getFields() const;
        private:
            std::vector<std::string> fields_;
            std::string swiz_;
    };
}
#endif
