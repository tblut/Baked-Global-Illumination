// This file is auto-generated and should not be modified directly.
#include "TextureCubeMap.hh"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "glow/data/SurfaceData.hh"
#include "glow/data/TextureData.hh"

#include "glow/glow.hh"
#include "glow/limits.hh"
#include "glow/common/runtime_assert.hh"
#include "glow/common/ogl_typeinfo.hh"
#include "glow/common/scoped_gl.hh"
#include "glow/common/thread_local.hh"

using namespace glow;

/// Currently bound buffer
static GLOW_THREADLOCAL TextureCubeMap::BoundTextureCubeMap *sCurrentTexture = nullptr;

TextureCubeMap::BoundTextureCubeMap *TextureCubeMap::getCurrentTexture()
{
    return sCurrentTexture;
}

bool TextureCubeMap::hasMipmapsEnabled() const
{
    switch (mMinFilter)
    {
    case GL_LINEAR_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_NEAREST_MIPMAP_NEAREST:
        return true;

    default:
        return false;
    }
}

GLenum TextureCubeMap::getUniformType() const
{
    auto fmt = getInternalFormat();

    if (isSignedIntegerInternalFormat(fmt))
        return GL_INT_SAMPLER_CUBE;
    else if (isUnsignedIntegerInternalFormat(fmt))
        return GL_UNSIGNED_INT_SAMPLER_CUBE;
    else if (isDepthInternalFormat(fmt))
        return mCompareMode == GL_NONE ? GL_SAMPLER_CUBE : GL_SAMPLER_CUBE_SHADOW;
    else
        return GL_SAMPLER_CUBE;
}

TextureCubeMap::TextureCubeMap (GLenum internalFormat)
  : Texture(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BINDING_CUBE_MAP, internalFormat)
{
}

SharedTextureCubeMap TextureCubeMap::create(int width, int height, GLenum internalFormat)
{
    auto tex = std::make_shared<TextureCubeMap>(internalFormat);
    tex->bind().resize(width, height);
    return tex;
}

SharedTextureCubeMap TextureCubeMap::createStorageImmutable(int width, int height, GLenum internalFormat, int mipmapLevels)
{
    auto tex = std::make_shared<TextureCubeMap>(internalFormat);
    tex->bind().makeStorageImmutable(width, height, internalFormat, mipmapLevels);
    return tex;
}

SharedTextureCubeMap TextureCubeMap::createFromFile(const std::string &filename, ColorSpace colorSpace)
{
    auto t = createFromData(TextureData::createFromFile(filename, colorSpace));
    t->setObjectLabel(filename);
    return t;
}

SharedTextureCubeMap TextureCubeMap::createFromFile(const std::string &filename, GLenum internalFormat, ColorSpace colorSpace)
{
    auto t = createFromData(TextureData::createFromFile(filename, colorSpace), internalFormat);
    t->setObjectLabel(filename);
    return t;
}

SharedTextureCubeMap TextureCubeMap::createFromData(const SharedTextureData &data)
{
    if (!data)
    {
        error() << "TextureCubeMap::createFromData failed, no data provided";
        return nullptr;
    }

    if (data->getPreferredInternalFormat() == GL_INVALID_ENUM)
    {
        error() << "TextureCubeMap::createFromData failed, no preferred internal format specified";
        return nullptr;
    }

    auto tex = std::make_shared<TextureCubeMap>(data->getPreferredInternalFormat());
    tex->bind().setData(data->getPreferredInternalFormat(), data);
    return tex;
}

SharedTextureCubeMap TextureCubeMap::createFromData(const SharedTextureData &data, GLenum internalFormat)
{
    if (!data)
    {
        error() << "TextureCubeMap::createFromData failed, no data provided";
        return nullptr;
    }

    auto tex = std::make_shared<TextureCubeMap>(internalFormat);
    tex->bind().setData(internalFormat, data);
    return tex;
}

bool TextureCubeMap::BoundTextureCubeMap::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentTexture == this, "Currently bound FBO does NOT match represented Texture " << to_string(texture), return false);
    return true;
}

void TextureCubeMap::BoundTextureCubeMap::makeStorageImmutable(int width, int height, GLenum internalFormat, int mipmapLevels)
{
    if (!isCurrent())
        return;

    GLOW_RUNTIME_ASSERT(!texture->isStorageImmutable(), "Texture is already immutable " << to_string(texture), return );
    checkValidGLOW();

    if (mipmapLevels <= 0)
        mipmapLevels = glm::floor(glm::log2(glm::max((float)width, (float)height))) + 1;

    texture->mStorageImmutable = true;
    texture->mInternalFormat = internalFormat;
    texture->mWidth = width;
    texture->mHeight = height;
    glTexStorage2D(texture->mTarget, mipmapLevels, internalFormat, width, height);
}

void TextureCubeMap::BoundTextureCubeMap::setMinFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();


    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, filter);
    texture->mMinFilter = filter;
}

void TextureCubeMap::BoundTextureCubeMap::setMagFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, filter);
    texture->mMagFilter = filter;
}

void TextureCubeMap::BoundTextureCubeMap::setFilter(GLenum magFilter, GLenum minFilter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, magFilter);
    texture->mMinFilter = minFilter;
    texture->mMagFilter = magFilter;
}

void TextureCubeMap::BoundTextureCubeMap::setAnisotropicFiltering(GLfloat samples)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    samples = glm::clamp(samples, 1.f, limits::maxAnisotropy);
    glTexParameterf(texture->mTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, samples);
    texture->mAnisotropicFiltering = samples;
}

void TextureCubeMap::BoundTextureCubeMap::setBorderColor(glm::vec4 const& color)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterfv(texture->mTarget, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    texture->mBorderColor = color;
}

void TextureCubeMap::BoundTextureCubeMap::setWrapS(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrap);
    texture->mWrapS = wrap;
}
void TextureCubeMap::BoundTextureCubeMap::setWrapT(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrap);
    texture->mWrapT = wrap;
}

void TextureCubeMap::BoundTextureCubeMap::setWrap(GLenum wrapS, GLenum wrapT)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrapS);
    texture->mWrapS = wrapS;
    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrapT);
    texture->mWrapT = wrapT;
}

void TextureCubeMap::BoundTextureCubeMap::setCompareMode(GLenum mode)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_COMPARE_MODE, mode);
    texture->mCompareMode = mode;
}

void TextureCubeMap::BoundTextureCubeMap::setCompareFunc(GLenum func)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_COMPARE_FUNC, func);
    texture->mCompareFunc = func;
}

void TextureCubeMap::BoundTextureCubeMap::setDepthStencilMode(GLenum mode)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_DEPTH_STENCIL_TEXTURE_MODE, mode);
    texture->mDepthStencilMode = mode;
}

void TextureCubeMap::BoundTextureCubeMap::generateMipmaps()
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glGenerateMipmap(texture->mTarget);
    texture->mMipmapsGenerated = true;
}

void TextureCubeMap::BoundTextureCubeMap::setLodBias(float bias)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterf(texture->mTarget, GL_TEXTURE_LOD_BIAS, bias);
    texture->mLodBias = bias;
}

void TextureCubeMap::BoundTextureCubeMap::setBaseLevel(int lvl)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_BASE_LEVEL, lvl);
    texture->mBaseLevel = lvl;
}

void TextureCubeMap::BoundTextureCubeMap::setMaxLevel(int lvl)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MAX_LEVEL, lvl);
    texture->mMaxLevel = lvl;
}

void TextureCubeMap::BoundTextureCubeMap::setMinLod(float lvl)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterf(texture->mTarget, GL_TEXTURE_MIN_LOD, lvl);
    texture->mMinLod = lvl;
}

void TextureCubeMap::BoundTextureCubeMap::setMaxLod(float lvl)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterf(texture->mTarget, GL_TEXTURE_MAX_LOD, lvl);
    texture->mMaxLod = lvl;
}

void TextureCubeMap::BoundTextureCubeMap::resize(int width, int height)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    GLOW_RUNTIME_ASSERT(!texture->isStorageImmutable(), "Texture is storage immutable " << to_string(texture), return );

    texture->mWidth = width;
    texture->mHeight = height;

    GLenum format = GL_RGBA;
    auto isIntegerFormat = isIntegerInternalFormat(texture->mInternalFormat);
    switch (channelsOfInternalFormat(texture->mInternalFormat))
    {
    case 1:
        format = isIntegerFormat ? GL_RED_INTEGER : GL_RED;
        break;
    case 2:
        format = isIntegerFormat ? GL_RG_INTEGER : GL_RG;
        break;
    case 3:
        format = isIntegerFormat ? GL_RGB_INTEGER : GL_RGB;
        break;
    case 4:
        format = isIntegerFormat ? GL_RGBA_INTEGER : GL_RGBA;
        break;
    }
    switch (texture->mInternalFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
        format = GL_DEPTH_COMPONENT;
        break;
    }

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, texture->mInternalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, texture->mInternalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, texture->mInternalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, texture->mInternalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, texture->mInternalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, texture->mInternalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

    texture->mMipmapsGenerated = false;
}

void TextureCubeMap::BoundTextureCubeMap::setData(GLenum internalFormat, GLenum target, int width, int height, GLenum format, GLenum type, const GLvoid *data, int mipmapLevel)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    if (texture->isStorageImmutable())
    {
        assert(mipmapLevel == 0 && "not implemented for higher levels");
        GLOW_RUNTIME_ASSERT(texture->mWidth == width, "Texture is storage immutable and a wrong width was specified " << to_string(texture), return );
        GLOW_RUNTIME_ASSERT(texture->mHeight == height, "Texture is storage immutable and a wrong height was specified " << to_string(texture), return );
        GLOW_RUNTIME_ASSERT(texture->mInternalFormat == internalFormat,
                            "Texture is storage immutable and a wrong internal format was specified " << to_string(texture), return );
    }

    if (mipmapLevel == 0)
    {
    texture->mWidth = width;
    texture->mHeight = height;
        texture->mMipmapsGenerated = false;
        texture->mInternalFormat = internalFormat;
    }

    // assure proper pixel store parameter
    scoped::unpackSwapBytes   _p0(false);
    scoped::unpackLsbFirst    _p1(false);
    scoped::unpackRowLength   _p2(0);
    scoped::unpackImageHeight _p3(0);
    scoped::unpackSkipRows    _p4(0);
    scoped::unpackSkipPixels  _p5(0);
    scoped::unpackSkipImages  _p6(0);
    scoped::unpackAlignment   _p7(1); // tight

    if (texture->isStorageImmutable())
        glTexSubImage2D(target, mipmapLevel, 0, 0, width, height, format, type, data);
    else
        glTexImage2D(target, mipmapLevel, texture->mInternalFormat, width, height, 0, format, type, data);
}

void TextureCubeMap::BoundTextureCubeMap::setSubData(GLenum target, int x, int y, int width, int height, GLenum format, GLenum type, const GLvoid *data, int mipmapLevel)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    // assure proper pixel store parameter
    scoped::unpackSwapBytes   _p0(false);
    scoped::unpackLsbFirst    _p1(false);
    scoped::unpackRowLength   _p2(0);
    scoped::unpackImageHeight _p3(0);
    scoped::unpackSkipRows    _p4(0);
    scoped::unpackSkipPixels  _p5(0);
    scoped::unpackSkipImages  _p6(0);
    scoped::unpackAlignment   _p7(1); // tight

    glTexSubImage2D(target, mipmapLevel, x, y, width, height, format, type, data);
 }

void TextureCubeMap::BoundTextureCubeMap::setData(GLenum internalFormat, const SharedTextureData &data)
{
    if (!isCurrent())
        return;

    texture->mInternalFormat = internalFormat; // format first, then resize
    resize(data->getWidth(), data->getHeight());

    // set all level 0 surfaces
    for (auto const &surf : data->getSurfaces())
        if (surf->getMipmapLevel() == 0)
            setSubData(surf->getTarget(), surf->getOffsetX(), surf->getOffsetY(),
                       surf->getWidth(), surf->getHeight(),
                       surf->getFormat(), surf->getType(),
                       surf->getData().data(), surf->getMipmapLevel());

    // set parameters
    if (data->getAnisotropicFiltering() >= 1.f)
        setAnisotropicFiltering(data->getAnisotropicFiltering());
    if (data->getMinFilter() != GL_INVALID_ENUM)
        setMinFilter(data->getMinFilter());
    if (data->getMagFilter() != GL_INVALID_ENUM)
        setMagFilter(data->getMagFilter());

    // generate mipmaps
    if (texture->hasMipmapsEnabled())
        generateMipmaps();

    // set all level 1+ surfaces
    for (auto const &surf : data->getSurfaces())
        if (surf->getMipmapLevel() > 0)
            setSubData(surf->getTarget(), surf->getOffsetX(), surf->getOffsetY(),
                       surf->getWidth(), surf->getHeight(),
                       surf->getFormat(), surf->getType(),
                       surf->getData().data(), surf->getMipmapLevel());
}

void TextureCubeMap::clear(GLenum format, GLenum type, const GLvoid* data, int mipmapLevel)
{
    checkValidGLOW();
#if GLOW_OPENGL_VERSION >= 44

    if (OGLVersion.total < 44)
    {
        glow::warning() << "Using fallback for Texture::clear because OpenGL Version is lower than 4.4.";
        glow::warning() << "  This has (severe) performance implications (see #43) " << to_string(this);

        glow::error() << "Not implemented. " << to_string(this);
    }
    else
    {
        glClearTexImage(mObjectName, mipmapLevel, format, type, data);
    }
#else
    error() << "TextureCubeMap::clear is only supported for OpenGL 4.4+ " << to_string(this);
#endif
    mMipmapsGenerated = false;
}

std::vector<char> TextureCubeMap::BoundTextureCubeMap::getData(GLenum format, GLenum type, int mipmapLevel)
{
    if (!isCurrent())
        return {};
    checkValidGLOW();

    auto target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;

    size_t dataSize = channelsOfFormat(format) * sizeOfTypeInBytes(type);

    GLint sWidth;
    glGetTexLevelParameteriv(target, mipmapLevel, GL_TEXTURE_WIDTH, &sWidth);
    dataSize *= sWidth;

    GLint sHeight;
    glGetTexLevelParameteriv(target, mipmapLevel, GL_TEXTURE_HEIGHT, &sHeight);
    dataSize *= sHeight;

    std::vector<char> data(dataSize);
    getData(format, type, data.size(), data.data(), mipmapLevel);
    return data;
}

void TextureCubeMap::BoundTextureCubeMap::getData(GLenum format, GLenum type, size_t bufferSize, void *buffer, int mipmapLevel)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    // assure proper pixel store parameter
    scoped::packSwapBytes   _p0(false);
    scoped::packLsbFirst    _p1(false);
    scoped::packRowLength   _p2(0);
    scoped::packImageHeight _p3(0);
    scoped::packSkipRows    _p4(0);
    scoped::packSkipPixels  _p5(0);
    scoped::packSkipImages  _p6(0);
    scoped::packAlignment   _p7(1); // tight

    (void)bufferSize; // TODO: check me!
    glGetTexImage(texture->mTarget, mipmapLevel, format, type, buffer);
}

SharedTextureData TextureCubeMap::BoundTextureCubeMap::getTextureData(int maxMipLevel)
{
    if (!isCurrent())
        return nullptr;
    checkValidGLOW();

    // assure proper pixel store parameter
    scoped::packSwapBytes   _p0(false);
    scoped::packLsbFirst    _p1(false);
    scoped::packRowLength   _p2(0);
    scoped::packImageHeight _p3(0);
    scoped::packSkipRows    _p4(0);
    scoped::packSkipPixels  _p5(0);
    scoped::packSkipImages  _p6(0);
    scoped::packAlignment   _p7(1); // tight

    auto tex = std::make_shared<TextureData>();

    // format
    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE; // TODO: 16bit, 32bit types
    size_t bytesPerTexel = 4;
    auto isIntegerFormat = isIntegerInternalFormat(texture->mInternalFormat);
    switch (channelsOfInternalFormat(texture->mInternalFormat))
    {
    case 1:
        format = isIntegerFormat ? GL_RED_INTEGER : GL_RED;
        bytesPerTexel = 1;
        break;
    case 2:
        format = isIntegerFormat ? GL_RG_INTEGER : GL_RG;
        bytesPerTexel = 2;
        break;
    case 3:
        format = isIntegerFormat ? GL_RGB_INTEGER : GL_RGB;
        bytesPerTexel = 3;
        break;
    case 4:
        format = isIntegerFormat ? GL_RGBA_INTEGER : GL_RGBA;
        bytesPerTexel = 4;
        break;
    }

    // tex parameters
    // TODO

    // surfaces

    for (auto target = GL_TEXTURE_CUBE_MAP_POSITIVE_X; target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; ++target)

    for (auto lvl = 0; lvl <= maxMipLevel; ++lvl)
    {
        GLint w;
        GLint h;
        GLint d;
        GLenum internalFormat;
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_DEPTH, &d);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_INTERNAL_FORMAT, (GLint *)&internalFormat);


        if (w * h * d == 0)
            break; // no mipmaps any more

        if (lvl == 0)
        {
            tex->setWidth(w);
            tex->setHeight(h);
            tex->setDepth(d);
            tex->setPreferredInternalFormat(internalFormat);
        }

        auto surface = std::make_shared<SurfaceData>();
        surface->setWidth(w);
        surface->setHeight(h);
        surface->setDepth(d);

        surface->setMipmapLevel(lvl);
        surface->setTarget(target);

        surface->setFormat(format);
        surface->setType(type);

        std::vector<char> data;
        data.resize(bytesPerTexel * w * h * d);
        glGetTexImage(target, lvl, surface->getFormat(), surface->getType(), data.data());
        surface->setData(data);

        tex->addSurface(surface);
    }

    return tex;
}

void TextureCubeMap::BoundTextureCubeMap::writeToFile(const std::string &filename)
{
    getTextureData()->saveToFile(filename);
}


TextureCubeMap::BoundTextureCubeMap::BoundTextureCubeMap (TextureCubeMap *texture) : texture(texture)
{
    checkValidGLOW();
    glGetIntegerv(texture->mBindingTarget, &previousTexture);
    glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
    glBindTexture(texture->mTarget, texture->mObjectName);

    previousTexturePtr = sCurrentTexture;
    sCurrentTexture = this;
}

TextureCubeMap::BoundTextureCubeMap::BoundTextureCubeMap (TextureCubeMap::BoundTextureCubeMap &&rhs)
  : texture(rhs.texture), previousTexture(rhs.previousTexture), previousTexturePtr(rhs.previousTexturePtr)
{
    // invalidate rhs
    rhs.previousTexture = -1;
}

TextureCubeMap::BoundTextureCubeMap::~BoundTextureCubeMap ()
{
    if (previousTexture != -1) // if valid
    {
        checkValidGLOW();
        glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
        glBindTexture(texture->mTarget, previousTexture);
        sCurrentTexture = previousTexturePtr;
    }
}
