#pragma once

#include <vector>
#include <string>

#include "common/property.hh"
#include "common/shared.hh"

#include "ActionEntry.hh"

namespace aion
{
class ActionLabel;

AION_SHARED(class, ActionPackage);
/// Collection of labels + entries
class ActionPackage
{
private:
    bool mDeleteLabels = false;

    std::vector<ActionEntry> mEntries;
    std::vector<ActionLabel*> mLabels;

public:
    AION_GETTER(Entries);
    AION_GETTER(Labels);

public:
    ActionPackage(std::vector<ActionEntry> const& entries, std::vector<ActionLabel*> const& labels);
    ~ActionPackage();

    void saveToFile(std::string const& filename) const;
    static SharedActionPackage loadFromFile(std::string const& filename);

    /// gets a package of all entries
    static SharedActionPackage complete();
};

}
