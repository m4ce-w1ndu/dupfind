#pragma once
#include "dedup/core/types.hpp"

namespace dedup {
namespace actions {

class ActionExecutor {
public:
    static bool execute(const DuplicateGroup& group, ActionType action);
};

} // namespace actions
} // namespace dedup
