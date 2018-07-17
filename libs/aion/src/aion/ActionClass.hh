#pragma once

#include <cstdint>
#include <string>

#include "common/shared.hh"

namespace aion
{
class ActionLabel;

AION_SHARED(class, ActionTree);

/// Only valid for within parent tree
class Action
{
public:
    int64_t starttime = -1;
    int64_t endtime = -1;
    int64_t duration = -1;
    ActionLabel* label;
    Action* parent = nullptr;
    Action* firstChild = nullptr;
    Action* nextSibling = nullptr;
    WeakActionTree tree;

    std::string const& name() const;
};
}
