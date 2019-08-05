#ifndef FRAG_CMD_COMMAND_H_
#define FRAG_CMD_COMMAND_H_

#include "../Group.h"
#include "../Video.h"
#include "../ValueStore.h"

namespace frag {
    namespace cmd {
        class Command {
            public:
                Command(const std::string& name, const Address& trigger, std::vector<AddressOrValue> args);

                virtual void run(std::shared_ptr<ValueStore> store) const = 0;

                virtual void validate() const = 0;

                Address getTrigger() const;
                std::string getName() const;

            protected:
                void throwIncompatible() const;

                const std::string name_;
                const Address target_;
                const Address trigger_;
                std::vector<AddressOrValue> args_;
        };
    }
}

#endif
