#pragma once

#include <sstream>
#include <iostream>
#include <string>

namespace glow
{
/// Defines the type of log
enum class LogLevel
{
    /// Important status information, always logged
    Info = 0x1,
    /// Indicates something may be wrong or inadvisable
    /// (Uses error stream)
    Warning = 0x2,
    /// Indicates a clear error case, something that should not be done or have happened in the first case
    /// (Uses error stream)
    Error = 0x4,
    /// Lesser debug information, is not output in Relase/Deploy builds
    Debug = 0x8,
};

/// logging implementation detail
namespace internal
{
extern std::ostream* logStream;
extern std::ostream* logStreamError;
extern std::string logPrefix;
extern uint8_t logMask;

struct LogObject
{
    std::ostream* oss;

    LogObject(std::ostream* oss, LogLevel lvl);
    ~LogObject();
};

template <typename T>
std::string to_string(const T& value)
{
	std::ostringstream os;
	os << value;
	return os.str();
}

template <typename T>
LogObject const& operator<<(LogObject const& lo, T const& obj)
{
	using namespace std;

    if (lo.oss)
        *lo.oss << to_string(obj);
    return lo;
}
}

/// Sets the prefix for logging
///
/// Supports the following variables:
///   $t Timestamp hh:mm:ss
///   $l Loglevel Info, Debug, Warning, Error
///
/// Default prefix: "[$t][$l] "
void setLogPrefix(std::string const& prefix);

/// Sets the output stream used for logging
/// (nullptr means default)
void setLogStream(std::ostream* ossNormal, std::ostream* ossError);

/// Sets a mask for logs
/// Only unmasked logs are shown
void setLogMask(uint8_t mask);
void setLogMask(LogLevel mask);
uint8_t getLogMask();

/// returns the current log stream for non-error logs
std::ostream* getLogStream();

/// returns the current log stream for error logs
std::ostream* getLogStreamError();

/// Initiates a new log (automatically appends std::endl at the end of this line
internal::LogObject log(LogLevel lvl = LogLevel::Info);

/// same as log(LogLevel::Info)
inline internal::LogObject info()
{
    return log(LogLevel::Info);
}
/// same as log(LogLevel::Warning)
inline internal::LogObject warning()
{
    return log(LogLevel::Warning);
}
/// same as log(LogLevel::Error)
inline internal::LogObject error()
{
    return log(LogLevel::Error);
}
/// same as log(LogLevel::Debug)
inline internal::LogObject debug()
{
    return log(LogLevel::Debug);
}
}
