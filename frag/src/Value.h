#ifndef FRAG_VALUE_H_
#define FRAG_VALUE_H_

#include <vector>

namespace frag {
    class Value {
        public:
            Value();
            Value(bool v);
            Value(float v);
            Value(std::vector<float> v);

            bool getBool();
            float getFloat();
            int getInt();
            std::vector<float> getVec4();

        private:
            std::vector<float> value_;
    };
}

#endif

