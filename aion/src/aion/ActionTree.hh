#pragma once

#include <vector>

#include "common/property.hh"
#include "common/shared.hh"

#include "Action.hh"
#include "ActionEntry.hh"
#include "ActionClass.hh"

namespace aion
{
AION_SHARED(class, ActionTree);
class ActionTree
{
private:
    /// all saved actions. Careful, their adresses are pinned
    std::vector<Action> mActions;
    std::vector<Action*> mRoots;
    std::vector<ActionLabel*> mLabels;

public:
    AION_GETTER(Actions);
    AION_GETTER(Roots);
    AION_GETTER(Labels);

    size_t getActionCount() const { return mActions.size(); }
    ActionTree();

    /// Scopes to all actions of this label (may result in strange behavior for recursive labels)
    /// New tree is independent
    SharedActionTree scopeTo(ActionLabel* label);
    /// Scopes to all given actions (they are the new roots)
    /// New tree is independent and does not share Action*
    SharedActionTree scopeTo(std::vector<Action*> const& actions);

    /// Dumps the _complete_ tree into the provided oss
    void dump(std::ostream& oss) const;

public:
    /// constructs an action tree from a given recording
    /// all unterminated actions will be aligned to the last entry
    /// if entry stack goes below zero, it is invalid (behavior may change in the future)
    static SharedActionTree construct(std::vector<ActionEntry> const& entries, std::vector<ActionLabel*> const& labels);
};
}
