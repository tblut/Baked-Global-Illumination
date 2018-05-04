#include "LocationMapping.hh"

#include "glow/common/log.hh"

#include <cassert>
#include <atomic>

using namespace glow;

/// tracks creation of new locations
static std::atomic_int sLocationCounter;

LocationMapping::LocationMapping()
{
    // get new ID
    mIdx = sLocationCounter.fetch_add(1);
}

int LocationMapping::count() const
{
    auto cnt = 0;
    for (auto const &s : mLocations)
        if (!s.empty())
            ++cnt;
    return cnt;
}

std::vector<std::pair<std::string, int>> LocationMapping::getMap() const
{
    std::vector<std::pair<std::string, int>> map;
    auto cnt = mLocations.size();
    for (auto loc = 0u; loc < cnt; ++loc)
        if (!mLocations[loc].empty())
            map.push_back({mLocations[loc], loc});
    return map;
}

int LocationMapping::queryLocation(const std::string &name) const
{
    assert(!name.empty());

    auto cnt = mLocations.size();

    for (auto loc = 0u; loc < cnt; ++loc)
        if (mLocations[loc] == name)
            return loc;

    return -1;
}

GLuint LocationMapping::getOrAddLocation(const std::string &name)
{
    assert(!name.empty());

    auto cnt = mLocations.size();

    // check if contained directly
    for (auto loc = 0u; loc < cnt; ++loc)
        if (mLocations[loc] == name)
            return loc;

    // check if empty one available
    for (auto loc = 0u; loc < cnt; ++loc)
        if (mLocations[loc].empty())
        {
            mLocations[loc] = name;
            return loc;
        }

    // otherwise add at end
    mLocations.push_back(name);
    return cnt;
}

GLuint LocationMapping::getOrAddLocation(const std::string &name, GLuint newLoc)
{
    assert(!name.empty());

    auto cnt = mLocations.size();

    // check if contained directly
    for (auto loc = 0u; loc < cnt; ++loc)
        if (mLocations[loc] == name)
            return loc;

    // alloc for newloc
    while (newLoc >= mLocations.size())
        mLocations.push_back("");

    // check if newLoc can be used
    if (mLocations[newLoc].empty())
    {
        mLocations[newLoc] = name;
        return newLoc;
    }

    // check if empty one available
    for (auto loc = 0u; loc < cnt; ++loc)
        if (mLocations[loc].empty())
        {
            mLocations[loc] = name;
            return loc;
        }

    // otherwise add at end
    mLocations.push_back(name);
    return cnt;
}

GLuint LocationMapping::addLocation(const std::string &name)
{
    assert(!name.empty());

    auto loc = queryLocation(name);
    if (loc != -1)
    {
        error() << "LocationMapping::addLocation, name `" << name << "' already mapped to " << loc;
        return loc;
    }

    return getOrAddLocation(name);
}

void LocationMapping::negotiate(SharedLocationMapping &lhs, SharedLocationMapping &rhs, bool &changedLhs, bool &changedRhs, bool reportChangedPtr)
{
    changedLhs = false;
    changedRhs = false;

    if (lhs == rhs)
        return; // already equal, nop

    // use first created mapping as truth
    auto keepLhs = lhs->mIdx < rhs->mIdx;
    auto trueMapping = keepLhs ? lhs : rhs;
    auto otherMapping = keepLhs ? rhs : lhs;
    auto changed = keepLhs ? &changedRhs : &changedLhs;

    // insert all locations from "other" into "true"
    // changed is true iff any loc changed
    auto otherCnt = otherMapping->mLocations.size();
    for (auto otherLoc = 0u; otherLoc < otherCnt; ++otherLoc)
        if (!otherMapping->mLocations[otherLoc].empty())
        {
            auto trueLoc = trueMapping->getOrAddLocation(otherMapping->mLocations[otherLoc]);

            if (trueLoc != otherLoc)
                *changed = true;
        }

    // actually remap "other"
    if (keepLhs)
    {
        rhs = lhs;
        if (reportChangedPtr)
            changedRhs = true;
    }
    else
    {
        lhs = rhs;
        if (reportChangedPtr)
            changedLhs = true;
    }
}
