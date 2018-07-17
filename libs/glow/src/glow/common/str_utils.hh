#pragma once

#include <algorithm>
#include <sstream>
#include <string>

namespace glow
{
namespace util
{
/// Returns true iff str endswith suffix
inline bool endswith(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/// Returns the last file ending of a given filename
/// Includes the '.'
/// e.g. returns ".png" for "/path/to/myfile.foo.png"
inline std::string fileEndingOf(std::string const &str)
{
    auto minPosA = str.rfind('/');
    if (minPosA == std::string::npos)
        minPosA = 0;

    auto minPosB = str.rfind('\\');
    if (minPosB == std::string::npos)
        minPosB = 0;

    auto minPos = minPosA < minPosB ? minPosB : minPosA;

    auto dotPos = str.rfind('.');
    if (dotPos == std::string::npos)
        dotPos = 0;

    if (dotPos <= minPos)
        return ""; // no '.' in actual file name

    return str.substr(dotPos);
}
/// Converts the string to lower
inline std::string toLower(std::string const &str)
{
    auto data = str;
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);
    return data;
}
/// Converts the string to upper
inline std::string toUpper(std::string const &str)
{
    auto data = str;
    std::transform(data.begin(), data.end(), data.begin(), ::toupper);
    return data;
}

/// Returns the path of the given file without trailing /
inline std::string pathOf(std::string const &filename)
{
    auto minPosA = filename.rfind('/');
    if (minPosA == std::string::npos)
        minPosA = 0;

    auto minPosB = filename.rfind('\\');
    if (minPosB == std::string::npos)
        minPosB = 0;

    auto minPos = minPosA < minPosB ? minPosB : minPosA;
    if (minPos == 0)
        return "";
    return filename.substr(0, minPos);
}

/// strips everything after (and including) the last dot of the filename
/// returns "" otherwise
/// e.g. "a.b.c" -> "a.b" but "a.b/c" -> ""
inline std::string stripFileDot(std::string const &filename)
{
    auto dotPos = filename.rfind('.');
    if (dotPos == std::string::npos)
        return "";

    auto slashPosA = filename.rfind('/');
    if (slashPosA != std::string::npos && slashPosA > dotPos)
        return "";

    auto slashPosB = filename.rfind('\\');
    if (slashPosB != std::string::npos && slashPosB > dotPos)
        return "";

    return filename.substr(0, dotPos);
}

template <typename T>
std::string joinToString(T const &coll, std::string const &sep = ", ")
{
    std::ostringstream oss;
    auto first = true;
    for (auto const &o : coll)
    {
        if (first)
            first = false;
        else
            oss << sep;

        oss << o;
    }
    return oss.str();
}
}
}
