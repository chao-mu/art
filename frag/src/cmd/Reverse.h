#ifndef FRAG_CMD_REVERSE_H_
#define FRAG_CMD_REVERSE_H_

// Ours
#include "Command.h"
#include "../Video.h"

namespace frag {
    namespace cmd {
        class Reverse : public Command {
            public:
                using Command::Command;

                virtual void run(std::shared_ptr<ValueStore> store) const override;
                virtual void validate() const override;
        };
    }
}

#endif
