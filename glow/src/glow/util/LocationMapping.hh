#pragma once

#include <vector>

#include "glow/common/shared.hh"
#include "glow/common/non_copyable.hh"

#include "glow/gl.hh"

namespace glow
{
GLOW_SHARED(class, LocationMapping);

class LocationMapping final
{
    GLOW_NON_COPYABLE(LocationMapping);

    /// faster than map
    /// location is implicit
    /// unused location are empty strings
    std::vector<std::string> mLocations;

    /// Creation index, used for determining which mapping "wins"
    int mIdx = -1;

public:
    LocationMapping();

    /// Returns the number of non-empty entries
    /// Runs in O(n)
    int count() const;

    /// Creates the location map for reading
    std::vector<std::pair<std::string, int>> getMap() const;

    /// Returns the location of a given name
    /// -1 if location not found
    int queryLocation(std::string const& name) const;

    /// Returns the location of a given name
    /// Adds it if not found
    GLuint getOrAddLocation(std::string const& name);
    /// Returns the location of a given name
    /// Adds it if not found
    /// if 'newLoc' is free, it is used
    GLuint getOrAddLocation(std::string const& name, GLuint newLoc);

    /// Adds a name and returns the location
    /// Error if already contained
    GLuint addLocation(std::string const& name);

    /**
     * @brief negotiates two location mappings (i.e. unifies them both)
     * @param changedLhs true iff lhs changed value/location
     * @param changedRhs true iff rhs changed value/location
     */
    static void negotiate(SharedLocationMapping& lhs, SharedLocationMapping& rhs, bool& changedLhs, bool& changedRhs, bool reportChangedPtr = false);
};
}
