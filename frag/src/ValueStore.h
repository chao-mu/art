#ifndef FRAG_VALUESTORE_H_
#define FRAG_VALUESTORE_H_

// STL
#include <optional>
#include <map>
#include <memory>

// Ours
#include "Address.h"
#include "Value.h"
#include "Media.h"
#include "midi/Control.h"

namespace frag {
    class ValueStore {
        public:
            bool isMedia(const Address& addr) const;
            void setIsMedia(const Address& addr, bool is_media);

            std::optional<Value> getValue(const Address& addr) const;
            std::shared_ptr<Media> getMedia(const Address& addr) const;

            void set(Address addr, Value v);
            void set(Address addr, std::shared_ptr<Media> m);
            void set(Address addr, midi::Control c);

        private:
            std::map<Address, Value> values_;
            std::map<std::string, std::shared_ptr<Media>> media_;
            std::map<std::string, bool> is_media_;
    };
}

#endif

