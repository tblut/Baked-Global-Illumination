#pragma once

#include <vector>
#include <map>
#include <cmath>
#include <chrono>
#include <ostream>

#include "common/property.hh"
#include "common/shared.hh"

namespace aion
{
class Action;
class ActionLabel;
AION_SHARED(class, ActionTree);
AION_SHARED(class, ActionAnalyzer);

/// Light class for analyzing a number of actions
/// Does pin the tree so should be relatively resistant
/// Moment-based statistics are calculated immidiately, percentile-based ones are deferred (except min/max)
/// Should not be used/trusted with zero entries
/// Times are either in NS or Secs
class ActionAnalyzer
{
private:
    /// pinned tree
    SharedActionTree mTree;

    /// analyzed actions
    std::vector<Action*> mActions;

    /// true iff actions already sorted
    bool mSorted = false;

    /// statistical moments
    int64_t mCount;
    int64_t mSumNs;
    double mVariance;
    int64_t mMin;
    int64_t mMax;

public: // properties
    AION_GETTER(Tree);
    AION_GETTER(Actions);

    int64_t count() const { return mCount; }
    int64_t totalTimeNS() const { return mSumNs; }
    double totalTime() const { return mSumNs * 1.e-9; }
    double averageNS() const { return mCount == 0 ? 0LL : mSumNs / mCount; }
    double average() const { return averageNS() * 1.e-9; }
    double varianceNS() const { return mVariance; }
    double variance() const { return mVariance * 1.e-18; }
    double standardDeviationNS() const { return std::sqrt(mVariance); }
    double standardDeviation() const { return std::sqrt(mVariance) * 1.e-9; }
    int64_t minNS() const { return mMin; }
    double min() const { return mMin * 1.e-9; }
    int64_t maxNS() const { return mMax; }
    double max() const { return mMax * 1.e-9; }
    /// returns the p-th percentile (p in [0, 100])
    int64_t percentileNS(int p);
    double percentile(int p) { return percentileNS(p) * 1.e-9; }
    int64_t medianNS() { return percentileNS(50); }
    double median() { return medianNS() * 1.e-9; }    
public:
    /// generic constructor for a given set of actions
    ActionAnalyzer(SharedActionTree const& tree, std::vector<Action*> const& actions);
    ActionAnalyzer(SharedActionTree const& tree, std::vector<Action> const& actions);

    ActionAnalyzer(ActionAnalyzer&&) = default;     // move yes
    ActionAnalyzer(ActionAnalyzer const&) = delete; // copy no

    /// returns a new analyzer for every encountered label
    std::map<ActionLabel*, SharedActionAnalyzer> byLabel() const;

private:
    /// constructs a new analyzer without any actions at all
    ActionAnalyzer(SharedActionTree const& tree);

    /// sets actions and does basic math
    void setActions(std::vector<Action*> const& actions);
    void addAction(Action* a);
    void initActions();

    /// sorts the actions by duration if not already done
    void sortIfRequired();

public:
    /// dumps a summary of all actions into the given stream
    static void dumpSummary(std::ostream& oss, bool verbose);
};
}
