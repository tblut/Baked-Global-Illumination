#include "TextureData.hh"

#include <glow/common/log.hh>
#include <glow/common/profiling.hh>

using namespace glow;

SharedTextureData TextureData::loadWithGLI(const std::string &filename, const std::string &ending, ColorSpace colorSpace)
{
    GLOW_ACTION();

    error() << "Error loading GLI texture data from " << filename;
    error() << "  Reason: not implemented.";
    return nullptr;
}

void TextureData::saveWithGLI(TextureData *data, const std::string &filename, const std::string &ending)
{
    GLOW_ACTION();

    error() << "Error saving GLI texture data to " << filename;
    error() << "  Reason: not implemented.";
}
