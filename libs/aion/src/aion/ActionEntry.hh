#pragma once

#include <cstdint>

namespace aion
{
struct ActionEntry
{
    int32_t secs;
    int32_t nsecs;

    /// -1 if end of action
    /// Careful, this index is only valid in current program run
    int32_t labelIdx;

    int64_t timestamp() const { return secs * 1000000000LL + nsecs; }
};
}
