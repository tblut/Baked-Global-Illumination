#pragma once

/*
 * Three Timers are defined here: SystemTimer, ProcessTimer and ThreadTimer.
 * They can be used to do basic time measurements with high accuracy.
 *
 * SystemTimer is based on the time since 1.1.1970.
 *             NOTE: The system time might get changed during the application runtime (if someone sets the
 *             time manually).
 * ProcessTime is based on the runtime of the process/application.
 *             NOTE: This can be the CPU time used by the process so the time reported by this might differ
 *             from the 'clock at the wall' and thus is not a good basis for e.g. animations.
 * ThreadTime  is based on the time the thread has run in which it is called.
 *             NOTE: This can be the CPU time used by the thread so the time reported by this might differ
 *             from the 'clock at the wall' and thus is not a good basis for e.g. animations.
 *
 *
 * Just use it like:
 *
 * ProcessTime t; // automatic reset
 * slowFunction();
 * log() << "function took " << t.getTimeInSecondsD() << " seconds, ";
 * t.reset();
 * fastFunction();
 * log() << "other function took " << t.getTimeInNanoseconds() << " nanoseconds";
 */

#include <iostream>

#include <glow/common/log.hh>

// for the system independent fallback:
#include <ctime>

// C++11
#if (__cplusplus >= 201103L)
#include <chrono>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// System dependent includes
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(__linux__) || defined(__APPLE__)
#include <sys/time.h>
#elif defined(_WIN32)
// note: _WIN32 is also defined on 64 bit systems!
#endif


namespace glow
{
namespace timing
{
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Plattform independent timer code and fallbacks:
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// used to define the API for the System specific timers:
class PerformanceTimerInterface
{
public:
    PerformanceTimerInterface() {}
    virtual ~PerformanceTimerInterface() {}
    // set the mStartTime to the current time:
    inline void restart() { mStartTime = getTimeInNanoseconds(); }
    // returns the time in nanosecounds, a 64 bit unsigned int will overflow after 584 years...
    virtual uint64_t getTimeInNanoseconds() = 0;

    // 32 bit unsigned int with seconds will overflow after 136 years
    virtual uint32_t getTimeInSeconds() = 0;
    virtual double getTimeInSecondsD() = 0;

    // get time diff since last reset:
    virtual uint64_t getTimeDiffInNanoseconds() { return getTimeInNanoseconds() - mStartTime; }
    // 32 bit unsigned int with seconds will overflow after 136 years
    inline uint32_t getTimeDiffInSeconds() { return getTimeInSeconds() - (mStartTime / 1000000000); }
    inline double getTimeDiffInSecondsD() { return getTimeInSecondsD() - (mStartTime / 1000000000.0); }
    // get the system dependent resolution of the timing in nanoseconds
    // default implementation tests this, better timer implementations should
    // query this info from the system API!
    virtual uint64_t getTimerResolutionInNanoseconds()
    {
        uint64_t t0, t1;
        t0 = t1 = getTimeInNanoseconds();
        while (t0 == t1)
            t1 = getTimeInNanoseconds();
        return (t1 - t0);
    }

private:
    uint64_t mStartTime; // in nano seconds
};

//
// works in all C environments with varying resolution, does only get the time for the
// whole process
class CProcessTimer : public PerformanceTimerInterface
{
public:
    CProcessTimer() : PerformanceTimerInterface() { restart(); }
    ~CProcessTimer() {}
    // returns the CPU/Process/Thread time in nanosecounds, a 64 bit unsigned int will overflow after
    // 584 years...
    uint64_t getTimeInNanoseconds() { return clockToNanoseconds(clock()); }
    // 32 bit unsigned int with seconds will overflow after 136 years
    uint32_t getTimeInSeconds() { return (uint32_t)getTimeInSecondsD(); }
    double getTimeInSecondsD() { return (double)clock() / (double)CLOCKS_PER_SEC; }
private:
    uint64_t clockToNanoseconds(clock_t _clock)
    {
        double msec = (double)_clock / (double)CLOCKS_PER_SEC * (1000.0 * 1000.0 * 1000.0);
        return (uint64_t)msec;
    }
};

// fallback which gives a warning at runtime
class CSystemTimer : public CProcessTimer
{
public:
    CSystemTimer() : CProcessTimer()
    {
        static bool warned = false;
        if (!warned)
        {
            error() << "No system timer present on this OS - fallback to process time";
            warned = true;
        }
    }
};

// fallback which gives a warning at runtime
class CThreadTimer : public CProcessTimer
{
public:
    CThreadTimer() : CProcessTimer()
    {
        static bool warned = false;
        if (!warned)
        {
            error() << "No thread timer present on this OS - fallback to process time";
            warned = true;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// General Unix: Can be used on Linux or MacOS (probably other Unices as well) for the system time.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__linux__) || defined(__APPLE__)
class UnixSystemTimer : public PerformanceTimerInterface
{
public:
    UnixSystemTimer() : PerformanceTimerInterface() { restart(); }
    ~UnixSystemTimer() {}
    // returns the CPU/Process/Thread time in nanosecounds, a 64 bit unsigned int will overflow after
    // 584 years...
    uint64_t getTimeInNanoseconds()
    {
        timeval t;
        gettimeofday(&t, NULL);
        uint64_t time = t.tv_sec * 1000000000; // sec to nano
        time += t.tv_usec * 1000;              // micro to nano
        return time;
    }

    // 32 bit unsigned int with seconds will overflow after 136 years
    uint32_t getTimeInSeconds()
    {
        timeval t;
        gettimeofday(&t, NULL);
        return (uint32_t)(t.tv_sec);
    }

    double getTimeInSecondsD()
    {
        timeval t;
        gettimeofday(&t, NULL);
        double time = (double)t.tv_sec;
        time += t.tv_usec / 1000000.0; // micro to seconds
        return time;
    }
};
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Linux version is based on Posix timer, note that those are not present on OSX!
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __linux__

template <clockid_t TIMER_TYPE>
class PosixTimer : public PerformanceTimerInterface
{
public:
    PosixTimer() : PerformanceTimerInterface() { restart(); }
    ~PosixTimer() {}
    // set the mStartTime to the current time:
    // inline void restart() { timespec t; clock_gettime( TIMER_TYPE, &t); mStartTime = timespecTo64( t ); }

    // returns the CPU/Process/Thread time in nanosecounds, a 64 bit unsigned int will overflow after
    // 584 years...
    uint64_t getTimeInNanoseconds()
    {
        timespec t;
        clock_gettime(TIMER_TYPE, &t);
        return timespecTo64(t);
    }

    // 32 bit unsigned int with seconds will overflow after 136 years
    uint32_t getTimeInSeconds()
    {
        timespec t;
        clock_gettime(TIMER_TYPE, &t);
        return t.tv_sec;
    }
    double getTimeInSecondsD()
    {
        timespec t;
        clock_gettime(TIMER_TYPE, &t);
        return timespecToDouble(t);
    }

    // get the system dependent resolution of the timing in nanoseconds
    uint64_t getTimerResolutionInNanoseconds()
    {
        timespec t;
        clock_getres(TIMER_TYPE, &t);
        return timespecTo64(t);
    }

private:
    inline uint64_t timespecTo64(const timespec &_time) const { return (_time.tv_sec * 1000000000 + _time.tv_nsec); }
    inline double timespecToDouble(const timespec &_time) const
    {
        return (_time.tv_sec + _time.tv_nsec / 1000000000.0);
    }

    uint64_t mStartTime;
};

#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implementation selection based on OS:
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __linux__
typedef PosixTimer<((clockid_t)CLOCK_REALTIME)> SystemTimer;
typedef PosixTimer<((clockid_t)CLOCK_PROCESS_CPUTIME_ID)> ProcessTimer;
typedef PosixTimer<((clockid_t)CLOCK_THREAD_CPUTIME_ID)> ThreadTimer;


#elif defined(__APPLE__)

typedef UnixSystemTimer SystemTimer;
typedef CProcessTimer ProcessTimer;
typedef CThreadTimer ThreadTimer;

//#elif defined( _WIN32 )
// ToDo: on windows a good timer might be implementable with:
// QueryPerformanceFrequency
// QueryPerformanceCounter
// from #include <windows.h>

#else
//
// No good OS specific timers yet implemented / unknown OS, default to standart C versions:
//
typedef CSystemTimer SystemTimer;
typedef CProcessTimer ProcessTimer;
typedef CThreadTimer ThreadTimer;

#endif
}
}
