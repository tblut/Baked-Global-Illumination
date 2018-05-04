#pragma once

#include "Query.hh"

#include <glow/common/log.hh>

namespace glow
{
GLOW_SHARED(class, OcclusionQuery);
/**
 * Occlusion queries count the fragments that pass the z-test.
 *
 * There are two variants:
 * GL_SAMPLES_PASSED     - will count the fragments
 * GL_ANY_SAMPLES_PASSED - will just tell whether fragments have passed the z-test, not how many (0 or any number)
 */
class OcclusionQuery final : public Query
{
public:
    OcclusionQuery() : Query(GL_SAMPLES_PASSED) {}
    OcclusionQuery(GLenum _queryType) : Query(_queryType) { setType(_queryType); }
    /// _queryType has to be GL_SAMPLES_PASSED or GL_ANY_SAMPLES_PASSED
    void setType(GLenum _queryType)
    {
        if (_queryType != GL_SAMPLES_PASSED)
        {
            error() << "OcclusionQuery type " << _queryType << " not supported " << to_string(this);
            _queryType = GL_SAMPLES_PASSED;
        }
        mTarget = _queryType;
    }

    /// get the actual number of fragments, unless the type is GL_ANY_SAMPLES_PASSED, than it only tells 0 or any value
    GLuint samplesPassed() { return getResult(); }

public:
	static SharedOcclusionQuery create(GLenum _queryType = GL_SAMPLES_PASSED) { return std::make_shared<OcclusionQuery>(_queryType); }
};
}
