#ifndef FRAG_VALUESTORE_H_
#define FRAG_VALUESTORE_H_

// STL
#include <optional>
#include <map>
#include <memory>

// Ours
#include "AddressOrValue.h"
#include "Group.h"
#include "Media.h"
#include "midi/Control.h"

namespace frag {
    class ValueStore {
        public:
            bool isMedia(Address addr) const;
            void setIsMedia(Address addr, bool is_media);

            std::optional<Value> getValue(Address addr) const;
            std::shared_ptr<Media> getMedia(Address addr) const;

            void set(Address addr, Value v);
            void set(Address addr, std::shared_ptr<Media> m);
            void set(Address addr, midi::Control c);
            void set(Address addr, std::shared_ptr<Group> g);

            Address getAddress(Address addr) const;

        private:
            std::map<Address, Value> values_;
            std::map<Address, std::shared_ptr<Media>> media_;
            std::map<Address, bool> is_media_;
            std::map<Address, AddressOrValue> aovs_;
    };
}

#endif

