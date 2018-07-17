#pragma once

#include "glow/common/shared.hh"
#include "glow/common/non_copyable.hh"

#include "glow/gl.hh"

#include <string>
#include <vector>

namespace glow
{
GLOW_SHARED(class, UniformState);
GLOW_SHARED(class, Program);
/**
 * @brief The UniformState class contains a snapshot of uniforms (with their data) of a given program
 */
class UniformState final
{
    GLOW_NON_COPYABLE(UniformState);

public:
    /// Container for a given uniform
    /// Does NOT store location (because it is independent of that)
    struct Uniform
    {
        std::string name;
        GLenum type;
        GLint size; ///< size as-in nr of components
        std::vector<char> data;
    };

private:
    /// List of all uniforms
    std::vector<Uniform> mUniforms;

public: // getter
    std::vector<Uniform> const& getUniforms() const { return mUniforms; }
public:
    UniformState();

    /// Adds a uniform to the state
    /// CAUTION: does not check if name already exists!
    void addUniform(std::string const& name, GLenum type, GLint size, std::vector<char> const& data);

    /// Writes all stored values of this state
    /// Uses "name" for lookup
    void restore();

    /// Creates a uniform state from a given program
    /// (Reads all uniform data)
    static SharedUniformState create(Program const* prog);
};
}
