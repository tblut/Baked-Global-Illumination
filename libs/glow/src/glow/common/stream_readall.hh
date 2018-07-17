#pragma once

#include <fstream>

namespace glow
{
namespace util
{
/// Reads the complete content of "in" into a string
inline std::string readall(std::istream &in)
{
    std::string ret;
    char buffer[4096];
    while (in.read(buffer, sizeof(buffer)))
        ret.append(buffer, sizeof(buffer));
    ret.append(buffer, in.gcount());
    return ret;
}
}
}
