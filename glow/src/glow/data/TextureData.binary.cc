#include "TextureData.hh"

#include <fstream>

#include <glow/common/log.hh>
#include <glow/common/profiling.hh>

#include <snappy/snappy.h>

using namespace glow;

SharedTextureData TextureData::loadWithBinary(const std::string &filename, const std::string &ending, ColorSpace colorSpace)
{
    GLOW_ACTION();

    std::ifstream is(filename, std::ios::binary);
    if (!is.good())
    {
        error() << "Error loading '.texdata' texture data from " << filename;
        error() << "  Reason: file cannot be opened.";
        return nullptr;
    }

    // get length of file:
    is.seekg(0, std::ios::end);
    auto length = is.tellg();
    is.seekg(0, std::ios::beg);

    // read data
    std::vector<char> rawdata;
    rawdata.resize(length);
    is.read(rawdata.data(), rawdata.size());
    is.close();

    // uncompress
    // TODO

    error() << "Error loading '.texdata' texture data from " << filename;
    error() << "  Reason: Not implemented.";
    return nullptr;
}

void TextureData::saveWithBinary(TextureData *data, const std::string &filename, const std::string &ending)
{
    GLOW_ACTION();

    error() << "Error saving binary '.texdata'' texture data to " << filename;
    error() << "  Reason: not implemented.";
}
