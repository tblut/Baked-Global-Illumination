#pragma once

#include "Query.hh"

#include <glow/common/log.hh>

namespace glow
{
GLOW_SHARED(class, PrimitiveQuery);
/**
 * Primitive queries count the number of processed geometry. Sounds trivial as the app should
 * know the number from the glDraw* calls, but this query will also count geometry generated
 * by geometry/tessellation shaders.
 *
 * During transform feedback let one query of each type run and compare the results: if more
 * primitives were generated than written to the TF buffer, the buffer overflowd.
 */
class PrimitiveQuery final : public Query
{
public:
    PrimitiveQuery() : Query(GL_PRIMITIVES_GENERATED) {}
    PrimitiveQuery(GLenum _queryType) : Query(_queryType) { setType(_queryType); }
    /// _queryType has to be GL_PRIMITIVES_GENERATED or GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN
    void setType(GLenum _queryType)
    {
        if ((_queryType != GL_PRIMITIVES_GENERATED) && (_queryType != GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN))
        {
            error() << "PrimitiveQuery type " << _queryType << " not supported " << to_string(this);
            _queryType = GL_PRIMITIVES_GENERATED;
        }
        mTarget = _queryType;
    }

public:
	static SharedPrimitiveQuery create(GLenum _queryType = GL_PRIMITIVES_GENERATED) { return std::make_shared<PrimitiveQuery>(_queryType); }
};
}
