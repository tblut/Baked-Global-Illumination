#pragma once


#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "format.hh"

/// Contains constants for conversion from ns to other times
/// Examples: 10 * systime::ms are 10ms
namespace aion_systime
{
const int64_t h = 60LL * 60LL * 1000LL * 1000LL * 1000LL;
const int64_t min = 60LL * 1000LL * 1000LL * 1000LL;
const int64_t sec = 1000LL * 1000LL * 1000LL;
const int64_t ms = 1000LL * 1000LL;
const int64_t us = 1000LL;
const int64_t ns = 1LL;

/**
 * @brief formats a time in ns with thousands separator
 * @param ns time in ns
 * @param oss output stream
 */
inline void formatNs(int64_t _ns, std::ostream &oss)
{
    if (_ns < 0)
    {
        oss << "-";
        formatNs(-_ns, oss);
        return;
    }

    if (_ns < 1000)
    {
        oss << _ns;
        return;
    }

    std::vector<int> ps;
    while (_ns > 0)
    {
        ps.insert(ps.begin(), _ns % 1000);
        _ns /= 1000;
    }
    for (unsigned i = 0; i < ps.size(); ++i)
    {
        int nr = ps[i];
        if (i > 0)
        {
            oss << ",";
            if (nr >= 100)
                oss << nr;
            else if (nr >= 10)
                oss << "0" << nr;
            else
                oss << "00" << nr;
        }
        else
            oss << nr;
    }
}
/**
 * @brief formats a time in ns with thousands separator
 * @param ns input time in NS
 * @return formatted string
 */
inline std::string formatNs(int64_t ns)
{
    std::ostringstream oss;
    formatNs(ns, oss);
    return oss.str();
}
/// formats a human-readable version (e.g. sec, ms, ns, us)
inline void formatHuman(int64_t ns, std::ostream &oss)
{
    if (ns < 1 * us)
        oss << aion_fmt::format("{:6}ns", ns);
    else if (ns < 1 * ms)
        oss << aion_fmt::format("{:6.2f}us", ns / 1000.);
    else if (ns < 1 * sec)
        oss << aion_fmt::format("{:6.2f}ms", ns / 1000. / 1000.);
    else
        oss << aion_fmt::format("{:6.2f}s ", ns / 1000. / 1000. / 1000.);
}
inline std::string formatHuman(int64_t ns)
{
    std::ostringstream oss;
    formatHuman(ns, oss);
    return oss.str();
}
inline void formatHumanHtml(int64_t ns, std::ostream &oss)
{
    if (ns < 1 * us)
        oss << aion_fmt::format("<font color=\"#bbb\">{:}ns</font>", ns);
    else if (ns < 1 * ms)
        oss << aion_fmt::format("<font color=\"#686\">{:.2f}us</font>", ns / 1000.);
    else if (ns < 1 * sec)
        oss << aion_fmt::format("{:.2f}ms", ns / 1000. / 1000.);
    else
        oss << aion_fmt::format("<font color=\"#BB4500\">{:.2f}s</font> ", ns / 1000. / 1000. / 1000.);
}
inline std::string formatHumanHtml(int64_t ns)
{
    std::ostringstream oss;
    formatHumanHtml(ns, oss);
    return oss.str();
}
}
