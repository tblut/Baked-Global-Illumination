#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include "common/property.hh"

#include "ActionEntry.hh"

namespace aion
{
class ActionLabel
{
private:
    std::string mName;
    std::string mFile;
    int mLine;
    std::string mFunction;

    int32_t mIndex;

public:
    AION_GETTER(Index);

    AION_GETTER(Name);
    AION_GETTER(File);
    AION_GETTER(Line);
    AION_GETTER(Function);

    std::string shortDesc() const;

    std::string nameOrFunc() const;

public:
    ActionLabel(const char* file, int line, const char* function, const char* name);

    void startEntry();
    void endEntry();

    /// gets a COPY! of the vector of all labels (labels itself are not copied)
    static std::vector<ActionLabel*> getAllLabels();

    static int64_t getLastEntryIdx();
    /// inclusive
    static std::vector<ActionEntry> copyEntries(int64_t startIdx, int64_t endIdx);
    /// all threads
    static std::vector<ActionEntry> copyAllEntries();

private:
    ActionLabel(std::string const& name, std::string const& function, std::string const& file, int line, int idx);
    friend class ActionPackage;   
};
}
