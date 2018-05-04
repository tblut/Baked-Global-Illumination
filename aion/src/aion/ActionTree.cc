#include "ActionTree.hh"

#include <cassert>

#include "common/auto.hh"

#include "common/systime.hh"

using namespace aion;

namespace
{
void writeStart(Action *a, std::vector<ActionEntry> &entries)
{
    entries.push_back({int32_t(a->starttime / 1000000000LL), int32_t(a->starttime % 1000000000LL), a->label->getIndex()});
}
void writeEnd(Action *a, std::vector<ActionEntry> &entries)
{
    entries.push_back({int32_t(a->endtime / 1000000000LL), int32_t(a->endtime % 1000000000LL), -1});
}

void writeAction(Action *a, std::vector<ActionEntry> &entries)
{
    writeStart(a, entries);

    _ c = a->firstChild;
    while (c)
    {
        writeAction(c, entries);
        c = c->nextSibling;
    }

    writeEnd(a, entries);
}
}

ActionTree::ActionTree()
{
}

SharedActionTree ActionTree::scopeTo(ActionLabel *label)
{
    std::vector<Action *> actions;
    for (_ &a : mActions)
        if (a.label == label)
            actions.push_back(&a);
    return scopeTo(actions);
}

SharedActionTree ActionTree::scopeTo(const std::vector<Action *> &actions)
{
    std::vector<ActionEntry> entries;
    for (_ const &a : actions)
        writeAction(a, entries);
    return construct(entries, mLabels);
}

SharedActionTree ActionTree::construct(const std::vector<ActionEntry> &entries, const std::vector<ActionLabel *> &labels)
{
    ACTION("ActionTree::construct");

    _ tree = std::make_shared<ActionTree>();
    tree->mLabels = labels;

    _ actionCnt = 0;
    for (_ const &e : entries)
        if (e.labelIdx >= 0)
        {
            assert(e.labelIdx < (int)labels.size());
            ++actionCnt;
        }

    tree->mActions.resize(actionCnt);

    std::vector<Action *> actionStack;

    _ actionIdx = size_t{0};
    Action *prevAction = nullptr;
    int64_t lastTime = -1;
    for (_ const &e : entries)
    {
        // start action
        if (e.labelIdx >= 0)
        {
            assert(actionIdx < tree->mActions.size());
            _ a = &tree->mActions[actionIdx];
            a->tree = tree;
            ++actionIdx;

            a->starttime = e.timestamp();
            a->label = labels[e.labelIdx];

            // parent and 1st child
            if (!actionStack.empty())
            {
                a->parent = actionStack.back();
                if (!a->parent->firstChild)
                    a->parent->firstChild = a;
            }
            else
                tree->mRoots.push_back(a);

            // next sibling
            if (prevAction && prevAction->parent == a->parent)
                prevAction->nextSibling = a;

            actionStack.push_back(a);
            lastTime = a->starttime;
        }
        else // end action
        {
            assert(actionStack.size() > 0);
            _ a = actionStack.back();
            actionStack.pop_back();

            a->endtime = e.timestamp();
            a->duration = a->endtime - a->starttime;
            lastTime = a->endtime;
            assert(a->duration >= 0);

            prevAction = a;
        }
    }

    // closure
    while (!actionStack.empty())
    {
        _ a = actionStack.back();
        actionStack.pop_back();

        assert(a->starttime >= 0);
        assert(a->endtime == -1);

        a->endtime = lastTime;
        a->duration = a->endtime - a->starttime;
    }

    // sanity
    for (_ const &a : tree->mActions)
    {
        assert(a.starttime >= 0);
        assert(a.endtime >= 0);
        assert(a.duration >= 0);
        assert(a.label);
        if (a.firstChild)
            assert(a.firstChild->parent == &a);
    }

    return tree;
}

static void dumpAction(std::ostream &oss, Action *a, std::string const &prefix)
{
    oss << prefix << " - " << aion_systime::formatHuman(a->duration) << " (" << a->label->nameOrFunc() << ")\n";
    _ c = a->firstChild;
    _ cp = prefix + "   ";
    while (c)
    {
        dumpAction(oss, c, cp);
        c = c->nextSibling;
    }
}

void ActionTree::dump(std::ostream &oss) const
{
    _ totalTime = 0 * aion_systime::ns;
    for (_ r : mRoots)
        totalTime += r->duration;

    oss << "Total Time: " << aion_systime::formatHuman(totalTime) << "\n";
    for (_ r : mRoots)
        dumpAction(oss, r, "");
    oss.flush();
}
