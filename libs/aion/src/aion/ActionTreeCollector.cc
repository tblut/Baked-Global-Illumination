#include "ActionTreeCollector.hh"

#include "ActionLabel.hh"
#include "ActionTree.hh"

using namespace aion;

ActionTreeCollector::ActionTreeCollector()
{
    start();
}

void ActionTreeCollector::start()
{
    startIdx = ActionLabel::getLastEntryIdx() + 1;
}

void ActionTreeCollector::end()
{
    endIdx = ActionLabel::getLastEntryIdx();
}

SharedActionTree ActionTreeCollector::generateTree() const
{
    if (startIdx == -1)
        return nullptr;

    auto end = endIdx;

    if (end == -1)
        end = ActionLabel::getLastEntryIdx();

    if (end == -1)
        return nullptr;

    // important: entries first, labels 2nd
    auto entries = ActionLabel::copyEntries(startIdx, end);
    auto labels = ActionLabel::getAllLabels();
    return ActionTree::construct(entries, labels);
}
