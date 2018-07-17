#include "GlowActionLabel.hh"

#include <algorithm>
#include <iostream>

#include <aion/common/format.hh>
#include <aion/common/systime.hh>

#include <cassert>

#include <glow/objects/TimerQuery.hh>
#include <glow/common/thread_local.hh>

#include <mutex>
#include <vector>
#include <stack>
#include <queue>

using namespace glow;

namespace
{
struct QueryEntry
{
    int index;
    SharedTimerQuery queryStart;
    SharedTimerQuery queryEnd;
};

GLOW_THREADLOCAL std::vector<GlowActionLabel::Entry> *sEntries = nullptr;
GLOW_THREADLOCAL std::queue<QueryEntry> *sQueries = nullptr;
GLOW_THREADLOCAL std::vector<SharedTimerQuery> *sTimers = nullptr;
GLOW_THREADLOCAL std::stack<int> *sEntryStack = nullptr;
std::mutex sLabelLock;
std::vector<GlowActionLabel *> sLabels;
std::vector<std::vector<GlowActionLabel::Entry> *> sEntriesPerThread;

#if _MSC_VER
LARGE_INTEGER sFrequency; // null init
#endif

int64_t getTime()
{
#if _MSC_VER
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    uint32_t secs = int32_t(time.QuadPart / sFrequency.QuadPart);
    uint32_t nsecs = int32_t((time.QuadPart % sFrequency.QuadPart) * 1000000000LL / sFrequency.QuadPart);
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    uint32_t secs = t.tv_sec;
    uint32_t nsecs = t.tv_nsec;
#endif
    return secs * 1000000000ULL + nsecs;
}
}

std::string GlowActionLabel::shortDesc() const
{
    auto filename = mFile;
    if (filename.find('/') != std::string::npos)
        filename = filename.substr(filename.rfind('/') + 1);
    if (filename.find('\\') != std::string::npos)
        filename = filename.substr(filename.rfind('\\') + 1);

    auto name = mName;
    if (name.empty())
        name = nameOrFunc();
    else
        name = "\"" + name + "\"";

    return name + ", " + filename + ":" + std::to_string(mLine);
}

std::string GlowActionLabel::nameOrFunc() const
{
    auto name = mName;
    if (name.empty())
    {
        name = mFunction;
        name = name.substr(0, name.find('('));
        name = name.substr(name.rfind(' ') + 1);
        // TODO: more special cases
        name += "()";
    }

    return name;
}

GlowActionLabel::GlowActionLabel(const char *file, int line, const char *function, const char *name)
  : mName(name), mFile(file), mLine(line), mFunction(function)
{
    sLabelLock.lock();

#if _MSC_VER
    if (sFrequency.QuadPart == 0)
        QueryPerformanceFrequency(&sFrequency);
#endif

    mIndex = sLabels.size();
    sLabels.push_back(this);
    if (!sEntries)
    {
        sEntries = new std::vector<Entry>();
        sQueries = new std::queue<QueryEntry>();
        sTimers = new std::vector<SharedTimerQuery>();
        sEntryStack = new std::stack<int>();
        sEntriesPerThread.push_back(sEntries);
    }
    sLabelLock.unlock();
}

void GlowActionLabel::startEntry()
{
    int entryIdx = sEntries->size();
    sEntryStack->push(entryIdx);

    Entry e;
    e.label = this;
    e.timeStartCPU = getTime();
    e._queryEnd = getQuery();
    sEntries->push_back(e);

    QueryEntry qe;
    qe.index = entryIdx;
    qe.queryStart = getQuery();
    qe.queryEnd = e._queryEnd;
    qe.queryStart->saveTimestamp();
    sQueries->push(qe);
}

void GlowActionLabel::endEntry()
{
    assert(!sEntryStack->empty() && "stack empty");

    auto idx = sEntryStack->top();
    sEntryStack->pop();

    Entry &e = sEntries->at(idx);
    e.timeEndCPU = getTime();
    e._queryEnd->saveTimestamp();
    e._queryEnd = nullptr;
}

std::vector<GlowActionLabel *> GlowActionLabel::getAllLabels()
{
    sLabelLock.lock();
    auto labels = sLabels;
    sLabelLock.unlock();
    return labels;
}

void GlowActionLabel::update(bool force)
{
    if (!sQueries)
        return;

    while (!sQueries->empty())
    {
        auto const &qe = sQueries->front();

        auto const &e = sEntries->at(qe.index);
        if (e.timeEndCPU == 0)
            break; // not finished yet

        if (force || (qe.queryStart->isResultAvailable() && qe.queryEnd->isResultAvailable()))
        {
            auto &e = sEntries->at(qe.index);
            e.timeStartGPU = qe.queryStart->getSavedTimestamp();
            e.timeEndGPU = qe.queryEnd->getSavedTimestamp();

            releaseQuery(qe.queryStart);
            releaseQuery(qe.queryEnd);

            auto l = e.label;
            l->mEntriesMutex.lock();
            l->mEntries.push_back(e);
            l->mEntriesMutex.unlock();

            sQueries->pop();
        }
        else
            break;
    }
}

void GlowActionLabel::print(int maxLines)
{
    struct Result
    {
        double sumCPU;
        double sumGPU;
        double avgCPU;
        double avgGPU;
        int count;
        std::string name;
    };

    std::vector<Result> labels;
    sLabelLock.lock();
    for (auto const &l : sLabels)
    {
        double sumCPU = 0;
        double sumGPU = 0;
        int count = 0;

        l->mEntriesMutex.lock();
        for (auto e : l->mEntries)
        {
            if (!e.isValid())
                continue;

            sumCPU += e.durationCPU();
            sumGPU += e.durationGPU();
            ++count;
        }
        l->mEntriesMutex.unlock();

        if (count == 0)
            ++count;

        labels.push_back({sumCPU, sumGPU, sumCPU / count, sumGPU / count, count, l->shortDesc()});
    }
    sLabelLock.unlock();

    std::sort(begin(labels), end(labels), [](Result const &r, Result const &l)
              {
                  return std::max(r.sumGPU, r.sumCPU) > std::max(l.sumGPU, l.sumCPU);
              });

    if (labels.size() == 0)
        return;

    int wTime = 8;
    int wCount = 6;

    std::cout << aion_fmt::format("{:>" + std::to_string(wTime) + "} {:>" + std::to_string(wTime) + "} {:>" + std::to_string(wTime)
                                      + "} {:>" + std::to_string(wTime) + "} {:>" + std::to_string(wCount) + "} {}",
                                  "GPU Sum", "CPU Sum", "GPU", "CPU", "Cnt", "Name")
              << std::endl;
    for (auto i = 0u; i < labels.size(); ++i)
    {
        if (maxLines-- <= 0)
            break;

        auto const &r = labels[i];
        std::cout << aion_fmt::format("{:>" + std::to_string(wTime) + "} {:>" + std::to_string(wTime) + "} {:>" + std::to_string(wTime)
                                          + "} {:>" + std::to_string(wTime) + "} {:>" + std::to_string(wCount) + "} {}",
                                      aion_systime::formatHuman(r.sumGPU), aion_systime::formatHuman(r.sumCPU),
                                      aion_systime::formatHuman(r.avgGPU), aion_systime::formatHuman(r.avgCPU),
                                      std::to_string(r.count) + "x", r.name)
                  << std::endl;
    }
}

SharedTimerQuery GlowActionLabel::getQuery()
{
    if (sTimers->empty())
        return std::make_shared<TimerQuery>();
    else
    {
        SharedTimerQuery timer = sTimers->back();
        sTimers->pop_back();
        return timer;
    }
}

void GlowActionLabel::releaseQuery(const SharedTimerQuery &query)
{
    sTimers->push_back(query);
}
