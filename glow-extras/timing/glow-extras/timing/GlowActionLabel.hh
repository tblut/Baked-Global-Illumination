#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <mutex>

#include <glow/common/property.hh>
#include <glow/fwd.hh>

namespace glow
{
class GlowActionLabel
{
public:
    struct Entry
    {
        uint64_t timeStartCPU = 0;
        uint64_t timeStartGPU = 0;
        uint64_t timeEndCPU = 0;
        uint64_t timeEndGPU = 0;

        glow::SharedTimerQuery _queryEnd;
        GlowActionLabel* label = nullptr;

        int64_t durationCPU() const { return timeEndCPU - timeStartCPU; }
        int64_t durationGPU() const { return timeEndGPU - timeStartGPU; }
        bool isValid() const { return timeEndCPU != 0 && timeEndGPU != 0 && timeStartCPU != 0; }
    };

private:
    std::string mName;
    std::string mFile;
    int mLine;
    std::string mFunction;

    int32_t mIndex;

    std::vector<Entry> mEntries;
    std::mutex mEntriesMutex;

public:
    GLOW_GETTER(Index);

    GLOW_GETTER(Name);
    GLOW_GETTER(File);
    GLOW_GETTER(Line);
    GLOW_GETTER(Function);

    std::string shortDesc() const;

    std::string nameOrFunc() const;

public:
    GlowActionLabel(const char* file, int line, const char* function, const char* name);

    void startEntry();
    void endEntry();

    /// gets a COPY! of the vector of all labels (labels itself are not copied)
    static std::vector<GlowActionLabel*> getAllLabels();

    /// updates all labels of this thread
    /// should be called once per frame
    static void update(bool force = false);

    /// prints label summary to std::cout, does NOT update!
    static void print(int maxLines = 10);

private:
    static glow::SharedTimerQuery getQuery();
    static void releaseQuery(glow::SharedTimerQuery const& query);
};
}
