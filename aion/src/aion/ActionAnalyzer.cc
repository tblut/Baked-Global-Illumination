#include "ActionAnalyzer.hh"

#include <cassert>
#include <algorithm>

#include "ActionTree.hh"
#include "ActionLabel.hh"
#include "ActionClass.hh"

#include "common/auto.hh"
#include "common/format.hh"
#include "common/systime.hh"

using namespace aion;

int64_t ActionAnalyzer::percentileNS(int p)
{
    assert(0 <= p && p <= 100);
    sortIfRequired();

    _ idx = (mActions.size() - 1) * p / 100;

    assert(idx < mActions.size());

    return mActions[idx]->duration;
}

ActionAnalyzer::ActionAnalyzer(const SharedActionTree &tree, const std::vector<Action *> &actions)
  : ActionAnalyzer(tree)
{
    setActions(actions);
}


ActionAnalyzer::ActionAnalyzer(const SharedActionTree &tree, const std::vector<Action> &actions) : ActionAnalyzer(tree)
{
    _ cnt = actions.size();
    std::vector<Action *> as;
    as.resize(cnt);
    for (_ i = 0u; i < cnt; ++i)
        as[i] = const_cast<Action *>(actions.data() + i); // we're really not changing it, promise!
    setActions(as);
}

std::map<ActionLabel *, SharedActionAnalyzer> ActionAnalyzer::byLabel() const
{
    std::map<ActionLabel *, SharedActionAnalyzer> result;

    for (_ a : mActions)
    {
        // create on demand
        if (!result.count(a->label))
            result[a->label] = SharedActionAnalyzer(new ActionAnalyzer(mTree));

        // add action
        result[a->label]->addAction(a);
    }

    // init actions
    for (_ const &kvp : result)
        kvp.second->initActions();

    return result;
}

ActionAnalyzer::ActionAnalyzer(const SharedActionTree &tree) : mTree(tree)
{
}

void ActionAnalyzer::setActions(const std::vector<Action *> &actions)
{
    mActions.assign(actions.begin(), actions.end());
    initActions();
}

void ActionAnalyzer::addAction(Action *a)
{
    mActions.push_back(a);
}

void ActionAnalyzer::initActions()
{
    mCount = mActions.size();
    mSumNs = 0uLL;
    mVariance = 0.;
    mMin = std::numeric_limits<decltype(mMin)>::max();
    mMax = std::numeric_limits<decltype(mMax)>::min();

    if (mCount == 0)
        return;

    // avg, min, max
    for (_ a : mActions)
    {
        mSumNs += a->duration;
        if (a->duration > mMax)
            mMax = a->duration;
        if (a->duration < mMin)
            mMin = a->duration;
    }
    _ avg = mSumNs / double(mCount);

    // variance
    for (_ a : mActions)
    {
        _ diff = a->duration - avg;
        mVariance += diff * diff;
    }
}

void ActionAnalyzer::sortIfRequired()
{
    if (mSorted)
        return;

    std::sort(begin(mActions), end(mActions), [](Action *l, Action *r)
              {
                  return l->duration < r->duration;
              });

    mSorted = true;
}

void ActionAnalyzer::dumpSummary(std::ostream &oss, bool verbose)
{
    const _ verboseMax = 10u;

    // important: entries first, labels 2nd
    _ allEntries = ActionLabel::copyAllEntries();
    _ allLabels = ActionLabel::getAllLabels();
    _ allTree = ActionTree::construct(allEntries, allLabels);
    ActionAnalyzer allAnalyzer(allTree, allTree->getActions());

    { // per label
        _ labelMap = allAnalyzer.byLabel();
        std::vector<std::pair<ActionLabel *, SharedActionAnalyzer>> labels(begin(labelMap), end(labelMap));
        _ cnt = verbose || labels.size() < verboseMax ? labels.size() : verboseMax;

        oss << "Labels (by total time):\n";
        std::sort(begin(labels), end(labels), [](std::pair<ActionLabel *, SharedActionAnalyzer> const &l,
                                                 std::pair<ActionLabel *, SharedActionAnalyzer> const &r)
                  {
                      return l.second->totalTimeNS() > r.second->totalTimeNS();
                  });
        _ nameLength = 0;
        _ cntLength = 0;
        for (_ i = 0u; i < cnt; ++i)
        {
            nameLength = std::max(nameLength, (int)labels[i].first->shortDesc().size());
            cntLength = std::max(cntLength, (int)aion_fmt::format("{}", labels[i].second->count()).size());
        }
        _ cntFmt = "{:" + aion_fmt::format("{}", cntLength) + "}";

        for (_ i = 0u; i < cnt; ++i)
        {
            _ l = labels[i].first;
            _ const &a = labels[i].second;
            _ desc = l->shortDesc();
            oss << "  " << desc << std::string(nameLength - desc.size(), ' ') << "   ";
            oss << aion_fmt::format(cntFmt, a->count()) << "x ";
            aion_systime::formatHuman(a->averageNS(), oss);
            oss << " = ";
            aion_systime::formatHuman(a->totalTimeNS(), oss);
            oss << " (";
            aion_systime::formatHuman(a->minNS(), oss);
            oss << " ~ ";
            aion_systime::formatHuman(a->maxNS(), oss);
            oss << ", ±";
            aion_systime::formatHuman(a->standardDeviationNS(), oss);
            oss << ")\n";
        }
    }
}
