#include "log.hh"

#include <ctime>
#include <iomanip>
#include <cassert>
#include <chrono>
#include <sstream>

std::ostream *glow::internal::logStream = nullptr;
std::ostream *glow::internal::logStreamError = nullptr;
std::string glow::internal::logPrefix = "[$t][$l] ";
uint8_t glow::internal::logMask = 0xFF;

void glow::setLogStream(std::ostream *ossNormal, std::ostream *ossError)
{
    internal::logStream = ossNormal;
    internal::logStreamError = ossError;
}

glow::internal::LogObject::LogObject(std::ostream *oss, LogLevel lvl) : oss(oss)
{
    auto prefix = logPrefix; // copy

    // replace $l
    auto pos = prefix.find("$l");
    if (pos != std::string::npos)
    {
        const char *stype = nullptr;
        switch (lvl)
        {
        case LogLevel::Info:
            stype = "Info";
            break;
        case LogLevel::Debug:
            stype = "Debug";
            break;
        case LogLevel::Error:
            stype = "Error";
            break;
        case LogLevel::Warning:
            stype = "Warning";
            break;
        }

        prefix.replace(pos, 2, stype);
    }

    // replace $t
    pos = prefix.find("$t");
    if (pos != std::string::npos)
    {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char timestr[10];
        std::strftime(timestr, sizeof(timestr), "%H:%M:%S", std::localtime(&now));

        prefix.replace(pos, 2, timestr);
    }

    *this << prefix;
}

glow::internal::LogObject::~LogObject()
{
    if (oss)
        *oss << std::endl;
}

void glow::setLogPrefix(const std::string &prefix)
{
    internal::logPrefix = prefix;
}

std::ostream *glow::getLogStreamError()
{
    return internal::logStreamError ? internal::logStreamError : &std::cerr;
}

std::ostream *glow::getLogStream()
{
    return internal::logStream ? internal::logStream : &std::cout;
}

glow::internal::LogObject glow::log(LogLevel lvl)
{
    if (!((uint8_t)lvl & internal::logMask))
        return {nullptr, lvl}; // masked

    switch (lvl)
    {
    case LogLevel::Info:
        return {getLogStream(), lvl};
    case LogLevel::Warning:
    case LogLevel::Error:
        return {getLogStreamError(), lvl};
    case LogLevel::Debug:
#ifdef GLOW_DEBUG
        return {getLogStream(), lvl};
#else
        return {nullptr, lvl}; // no output
#endif
    default:
        assert(0 && "unknown log level");
        return {nullptr, lvl}; // error, wrong log level
    }
}

void glow::setLogMask(uint8_t mask)
{
    internal::logMask = (uint8_t)mask;
}

void glow::setLogMask(glow::LogLevel mask)
{
    internal::logMask = (uint8_t)mask;
}

uint8_t glow::getLogMask()
{
    return internal::logMask;
}
