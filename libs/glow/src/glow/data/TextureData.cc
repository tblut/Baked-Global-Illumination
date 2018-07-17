#include "TextureData.hh"

#include "SurfaceData.hh"

#include <glow/common/profiling.hh>
#include <glow/common/log.hh>
#include <glow/common/str_utils.hh>

using namespace glow;

TextureData::LoadFromFileFunc TextureData::sLoadFunc = nullptr;

TextureData::TextureData()
{
}

void TextureData::addSurface(const SharedSurfaceData &surface)
{
    mSurfaces.push_back(surface);
}

SharedTextureData TextureData::createFromFile(const std::string &filename, ColorSpace colorSpace)
{
    GLOW_ACTION();

    auto ending = util::toLower(util::fileEndingOf(filename));

    // try custom loader
    if (sLoadFunc)
    {
        auto tex = sLoadFunc(filename, ending, colorSpace);
        if (tex)
            return tex;
    }

    // lodepng
    if (ending == ".png")
        return loadWithLodepng(filename, ending, colorSpace);

    // stb
    if (ending == ".png" ||  //
        ending == ".jpg" ||  //
        ending == ".jpeg" || //
        ending == ".tga" ||  //
        ending == ".bmp" ||  //
        ending == ".psd" ||  //
        ending == ".gif" ||  //
        ending == ".hdr" ||  //
        ending == ".pic" ||  //
        ending == ".ppm" ||  //
        ending == ".pgm")
        return loadWithStb(filename, ending, colorSpace);

    // GLI
    if (ending == ".ktx" || //
        ending == ".dds")
        return loadWithGLI(filename, ending, colorSpace);

    // binary
    if (ending == ".texdata")
        return loadWithBinary(filename, ending, colorSpace);

// try Qt
#ifdef GLOW_USE_QT
    return loadWithQt(filename, ending, colorSpace);
#endif

    error() << "file type of `" << filename << "' not recognized.";
    return nullptr;
}

SharedTextureData TextureData::createFromFileCube(const std::string &fpx,
                                                  const std::string &fnx,
                                                  const std::string &fpy,
                                                  const std::string &fny,
                                                  const std::string &fpz,
                                                  const std::string &fnz,
                                                  ColorSpace colorSpace)
{
    GLOW_ACTION();

    auto tpx = createFromFile(fpx, colorSpace);
    auto tnx = createFromFile(fnx, colorSpace);
    auto tpy = createFromFile(fpy, colorSpace);
    auto tny = createFromFile(fny, colorSpace);
    auto tpz = createFromFile(fpz, colorSpace);
    auto tnz = createFromFile(fnz, colorSpace);

    SharedTextureData ts[] = {tpx, tnx, tpy, tny, tpz, tnz};
    for (auto i = 0; i < 6; ++i)
        if (!ts[i])
            return nullptr;

    for (auto i = 1; i < 6; ++i)
        if (ts[0]->getWidth() != ts[i]->getWidth() || ts[0]->getHeight() != ts[i]->getHeight())
        {
            std::string files[] = {fpx, fnx, fpy, fny, fpz, fnz};
            error() << "CubeMaps require same size for every texture: ";
            error() << "  " << ts[0]->getWidth() << "x" << ts[0]->getHeight() << ", " << files[0];
            error() << "  " << ts[i]->getWidth() << "x" << ts[i]->getHeight() << ", " << files[i];
            return nullptr;
        }

    auto tex = tpx;
    tex->setTarget(GL_TEXTURE_CUBE_MAP);
    for (auto const &s : tex->getSurfaces())
        s->setTarget(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    for (auto i = 1; i < 6; ++i)
        for (auto const &s : ts[i]->getSurfaces())
        {
            s->setTarget(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
            tex->addSurface(s);
        }
    return tex;
}

void TextureData::saveToFile(const std::string &filename, int quality)
{
    GLOW_ACTION();

    auto ending = util::toLower(util::fileEndingOf(filename));

    // TODO: 3D texture
    if (mDepth > 1)
    {
        warning() << "Texture has " << mDepth << " layers, only first one will be saved to " << filename;
    }

    // lodepng
    if (ending == ".png")
    {
        saveWithLodepng(this, filename, ending);
        return;
    }

    // stb
    if (ending == ".tga" || //
        ending == ".bmp" || //
        ending == ".hdr")
    {
        saveWithStb(this, filename, ending);
        return;
    }

    // gli
    if (ending == ".ktx" || //
        ending == ".dds")
    {
        saveWithGLI(this, filename, ending);
        return;
    }

    // binary
    if (ending == ".texdata")
    {
        saveWithBinary(this, filename, ending);
        return;
    }

// try Qt
#ifdef GLOW_USE_QT
    {
        saveWithQt(this, filename, ending);
        return;
    }
#endif

    error() << "file type of `" << filename << "' not recognized.";
    return;
}

void TextureData::setLoadFunction(const TextureData::LoadFromFileFunc &func)
{
    sLoadFunc = func;
}
