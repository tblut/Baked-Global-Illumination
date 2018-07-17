#pragma once

#include "common/shared.hh"

namespace aion
{
AION_SHARED(class, ActionTree);

/// ONLY WORKS IN SAME THREAD
class ActionTreeCollector
{
private:
    int64_t startIdx = -1;
    int64_t endIdx = -1;

public:
    /// implicitly calls start
    ActionTreeCollector();

    /// start collecting (can be called multiple times)
    /// does not include currently active actions
    void start();
    /// end collecting (can be called multiple times)
    void end();

    /// generate collected tree
    /// uses current last idx of end not called
    SharedActionTree generateTree() const;
};
}
