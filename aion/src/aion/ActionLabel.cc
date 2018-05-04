#include "ActionLabel.hh"

#include "common/format.hh"

#include <string>
#include <algorithm>
#include <vector>
#include <mutex>

#include "ActionEntry.hh"

using namespace aion;

#ifdef _MSC_VER
#include <Windows.h>
#define AION_THREADLOCAL __declspec(thread)
#else
#define AION_THREADLOCAL __thread // GCC 4.7 has no thread_local yet
#endif

namespace
{
AION_THREADLOCAL std::vector<ActionEntry> *sEntries = nullptr;
std::mutex sLabelLock;
std::vector<ActionLabel *> sLabels;
std::vector<std::vector<ActionEntry> *> sEntriesPerThread;

#if _MSC_VER
LARGE_INTEGER sFrequency; // null init
#endif

void writeTime(ActionEntry &e)
{
#if _MSC_VER
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    e.secs = int32_t(time.QuadPart / sFrequency.QuadPart);
    e.nsecs = int32_t((time.QuadPart % sFrequency.QuadPart) * 1000000000LL / sFrequency.QuadPart);
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    e.secs = t.tv_sec;
    e.nsecs = t.tv_nsec;
#endif
}
}

std::string ActionLabel::shortDesc() const
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

    return aion_fmt::format("{}, {}:{}", name, filename, mLine);
}

std::string ActionLabel::nameOrFunc() const
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

ActionLabel::ActionLabel(const char *file, int line, const char *function, const char *name)
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
        sEntries = new std::vector<ActionEntry>();
        sEntriesPerThread.push_back(sEntries);
    }
    sLabelLock.unlock();
}

void ActionLabel::startEntry()
{
    ActionEntry e;
    e.labelIdx = mIndex;
    writeTime(e);
    if (!sEntries)
    {
        sLabelLock.lock();
        if (!sEntries)
        {
            sEntries = new std::vector<ActionEntry>();
            sEntriesPerThread.push_back(sEntries);
        }
        sLabelLock.unlock();
    }
    sEntries->push_back(e);
}

void ActionLabel::endEntry()
{
    ActionEntry e;
    e.labelIdx = -1; // end
    writeTime(e);
    sEntries->push_back(e);
}

std::vector<ActionLabel *> ActionLabel::getAllLabels()
{
    sLabelLock.lock();
    auto copy = sLabels;
    sLabelLock.unlock();
    return copy;
}

int64_t ActionLabel::getLastEntryIdx()
{
    return sEntries ? (int64_t)sEntries->size() - 1 : -1;
}

std::vector<ActionEntry> ActionLabel::copyEntries(int64_t startIdx, int64_t endIdx)
{
    if (startIdx < 0 || startIdx > endIdx)
        return {};
    if (!sEntries)
        return {};

    return std::vector<ActionEntry>(sEntries->begin() + startIdx, sEntries->begin() + endIdx + 1);
}

std::vector<ActionEntry> ActionLabel::copyAllEntries()
{
    std::vector<ActionEntry> entries;
    sLabelLock.lock();
    for (auto const &pes : sEntriesPerThread)
        entries.insert(end(entries), begin(*pes), end(*pes));
    sLabelLock.unlock();
    return entries;
}

ActionLabel::ActionLabel(const std::string &name, const std::string &function, const std::string &file, int line, int idx)
  : mName(name), mFile(file), mLine(line), mFunction(function), mIndex(idx)
{
}
