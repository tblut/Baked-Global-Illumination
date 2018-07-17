#pragma once

#include "Query.hh"

namespace glow
{
GLOW_SHARED(class, TimerQuery);
/**
 * TimerQueries can get the GPU timestamp and measure GPU execution speed.
 *
 * Only available since OpenGL 3.3 or GL_ARB_timer_query (on OpenGL 3.2)
 */
class TimerQuery : public Query
{
public:
    TimerQuery() : Query(GL_TIME_ELAPSED) {}
    /// Mark the moment in the pipeline of which the time should get queried.
    void saveTimestamp() { glQueryCounter(mObjectName, GL_TIMESTAMP); }
    /// Get the current GPU timestamp.
    GLint64 getCurrentTimestamp()
    {
        GLint64 time;
        glGetInteger64v(GL_TIMESTAMP, &time);
        return time;
    }

    /// Get the timestamp saved by 'saveTimestamp'.
    GLuint64 getSavedTimestamp() { return getResult64(); }
    /// Converts a ns time value to seconds
    static double toSeconds(GLint64 timeInNs) { return timeInNs / (1000. * 1000. * 1000.); }

public:
	static SharedTimerQuery create() { return std::make_shared<TimerQuery>(); }
};
}
