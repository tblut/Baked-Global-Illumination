// This file is auto-generated and should not be modified directly.
#include "TextureRectangle.hh"

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
static GLOW_THREADLOCAL TextureRectangle::BoundTextureRectangle *sCurrentTexture = nullptr;

TextureRectangle::BoundTextureRectangle *TextureRectangle::getCurrentTexture()
{
    return sCurrentTexture;
}


GLenum TextureRectangle::getUniformType() const
{
    auto fmt = getInternalFormat();

    if (isSignedIntegerInternalFormat(fmt))
        return GL_INT_SAMPLER_2D_RECT;
    else if (isUnsignedIntegerInternalFormat(fmt))
        return GL_UNSIGNED_INT_SAMPLER_2D_RECT;
    else if (isDepthInternalFormat(fmt))
        return mCompareMode == GL_NONE ? GL_SAMPLER_2D_RECT : GL_SAMPLER_2D_RECT_SHADOW;
    else
        return GL_SAMPLER_2D_RECT;
}

TextureRectangle::TextureRectangle (GLenum internalFormat)
  : Texture(GL_TEXTURE_RECTANGLE, GL_TEXTURE_BINDING_RECTANGLE, internalFormat)
{
}

SharedTextureRectangle TextureRectangle::create(int width, int height, GLenum internalFormat)
{
    auto tex = std::make_shared<TextureRectangle>(internalFormat);
    tex->bind().resize(width, height);
    return tex;
}

SharedTextureRectangle TextureRectangle::createStorageImmutable(int width, int height, GLenum internalFormat)
{
    auto tex = std::make_shared<TextureRectangle>(internalFormat);
    tex->bind().makeStorageImmutable(width, height, internalFormat);
    return tex;
}

SharedTextureRectangle TextureRectangle::createFromFile(const std::string &filename, ColorSpace colorSpace)
{
    auto t = createFromData(TextureData::createFromFile(filename, colorSpace));
    t->setObjectLabel(filename);
    return t;
}

SharedTextureRectangle TextureRectangle::createFromFile(const std::string &filename, GLenum internalFormat, ColorSpace colorSpace)
{
    auto t = createFromData(TextureData::createFromFile(filename, colorSpace), internalFormat);
    t->setObjectLabel(filename);
    return t;
}

SharedTextureRectangle TextureRectangle::createFromData(const SharedTextureData &data)
{
    if (!data)
    {
        error() << "TextureRectangle::createFromData failed, no data provided";
        return nullptr;
    }

    if (data->getPreferredInternalFormat() == GL_INVALID_ENUM)
    {
        error() << "TextureRectangle::createFromData failed, no preferred internal format specified";
        return nullptr;
    }

    auto tex = std::make_shared<TextureRectangle>(data->getPreferredInternalFormat());
    tex->bind().setData(data->getPreferredInternalFormat(), data);
    return tex;
}

SharedTextureRectangle TextureRectangle::createFromData(const SharedTextureData &data, GLenum internalFormat)
{
    if (!data)
    {
        error() << "TextureRectangle::createFromData failed, no data provided";
        return nullptr;
    }

    auto tex = std::make_shared<TextureRectangle>(internalFormat);
    tex->bind().setData(internalFormat, data);
    return tex;
}

bool TextureRectangle::BoundTextureRectangle::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentTexture == this, "Currently bound FBO does NOT match represented Texture " << to_string(texture), return false);
    return true;
}

void TextureRectangle::BoundTextureRectangle::makeStorageImmutable(int width, int height, GLenum internalFormat)
{
    if (!isCurrent())
        return;

    GLOW_RUNTIME_ASSERT(!texture->isStorageImmutable(), "Texture is already immutable " << to_string(texture), return );
    checkValidGLOW();

    texture->mStorageImmutable = true;
    texture->mInternalFormat = internalFormat;
    texture->mWidth = width;
    texture->mHeight = height;
    glTexStorage2D(texture->mTarget, 1, internalFormat, width, height);
}

void TextureRectangle::BoundTextureRectangle::setMinFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    auto corrected = false;
    switch (filter) {
        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_NEAREST_MIPMAP_LINEAR:
            filter = GL_NEAREST;
            corrected = true;
            break;
        case GL_LINEAR_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_LINEAR:
            filter = GL_LINEAR;
            corrected = true;
            break;
    }
    if (corrected)
        warning() << "TextureRectangle does not support MipMapping. " << to_string(texture);

    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, filter);
    texture->mMinFilter = filter;
}

void TextureRectangle::BoundTextureRectangle::setMagFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, filter);
    texture->mMagFilter = filter;
}

void TextureRectangle::BoundTextureRectangle::setFilter(GLenum magFilter, GLenum minFilter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, magFilter);
    texture->mMinFilter = minFilter;
    texture->mMagFilter = magFilter;
}

void TextureRectangle::BoundTextureRectangle::setAnisotropicFiltering(GLfloat samples)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    samples = glm::clamp(samples, 1.f, limits::maxAnisotropy);
    glTexParameterf(texture->mTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, samples);
    texture->mAnisotropicFiltering = samples;
}

void TextureRectangle::BoundTextureRectangle::setBorderColor(glm::vec4 const& color)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterfv(texture->mTarget, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    texture->mBorderColor = color;
}

void TextureRectangle::BoundTextureRectangle::setWrapS(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrap);
    texture->mWrapS = wrap;
}
void TextureRectangle::BoundTextureRectangle::setWrapT(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrap);
    texture->mWrapT = wrap;
}

void TextureRectangle::BoundTextureRectangle::setWrap(GLenum wrapS, GLenum wrapT)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrapS);
    texture->mWrapS = wrapS;
    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrapT);
    texture->mWrapT = wrapT;
}

void TextureRectangle::BoundTextureRectangle::setCompareMode(GLenum mode)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_COMPARE_MODE, mode);
    texture->mCompareMode = mode;
}

void TextureRectangle::BoundTextureRectangle::setCompareFunc(GLenum func)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_COMPARE_FUNC, func);
    texture->mCompareFunc = func;
}

void TextureRectangle::BoundTextureRectangle::setDepthStencilMode(GLenum mode)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_DEPTH_STENCIL_TEXTURE_MODE, mode);
    texture->mDepthStencilMode = mode;
}


void TextureRectangle::BoundTextureRectangle::resize(int width, int height)
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

    glTexImage2D(texture->mTarget, 0, texture->mInternalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

}

void TextureRectangle::BoundTextureRectangle::setData(GLenum internalFormat, int width, int height, GLenum format, GLenum type, const GLvoid *data)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    if (texture->isStorageImmutable())
    {
        GLOW_RUNTIME_ASSERT(texture->mWidth == width, "Texture is storage immutable and a wrong width was specified " << to_string(texture), return );
        GLOW_RUNTIME_ASSERT(texture->mHeight == height, "Texture is storage immutable and a wrong height was specified " << to_string(texture), return );
        GLOW_RUNTIME_ASSERT(texture->mInternalFormat == internalFormat,
                            "Texture is storage immutable and a wrong internal format was specified " << to_string(texture), return );
    }

    texture->mWidth = width;
    texture->mHeight = height;
    texture->mInternalFormat = internalFormat;

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
        glTexSubImage2D(texture->mTarget, 0, 0, 0, width, height, format, type, data);
    else
        glTexImage2D(texture->mTarget, 0, texture->mInternalFormat, width, height, 0, format, type, data);
}

void TextureRectangle::BoundTextureRectangle::setSubData(int x, int y, int width, int height, GLenum format, GLenum type, const GLvoid *data)
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

    glTexSubImage2D(texture->mTarget, 0, x, y, width, height, format, type, data);
 }

void TextureRectangle::BoundTextureRectangle::setData(GLenum internalFormat, const SharedTextureData &data)
{
    if (!isCurrent())
        return;

    texture->mInternalFormat = internalFormat; // format first, then resize
    resize(data->getWidth(), data->getHeight());

    // set all level 0 surfaces
    for (auto const &surf : data->getSurfaces())
        if (surf->getMipmapLevel() == 0)
            setSubData(surf->getOffsetX(), surf->getOffsetY(),
                       surf->getWidth(), surf->getHeight(),
                       surf->getFormat(), surf->getType(),
                       surf->getData().data());

    // set parameters
    if (data->getAnisotropicFiltering() >= 1.f)
        setAnisotropicFiltering(data->getAnisotropicFiltering());
    if (data->getMinFilter() != GL_INVALID_ENUM)
        setMinFilter(data->getMinFilter());
    if (data->getMagFilter() != GL_INVALID_ENUM)
        setMagFilter(data->getMagFilter());
    if (data->getWrapS() != GL_INVALID_ENUM)
        setWrapS(data->getWrapS());
    if (data->getWrapT() != GL_INVALID_ENUM)
        setWrapT(data->getWrapT());

}

void TextureRectangle::clear(GLenum format, GLenum type, const GLvoid* data)
{
    checkValidGLOW();
#if GLOW_OPENGL_VERSION >= 44

    if (OGLVersion.total < 44)
    {
        glow::warning() << "Using fallback for Texture::clear because OpenGL Version is lower than 4.4.";
        glow::warning() << "  This has (severe) performance implications (see #43) " << to_string(this);

        // assemble img
        std::vector<uint8_t> rawdata;
        int stride = channelsOfFormat(format) * sizeOfTypeInBytes(type);
        int w = mWidth;
        int h = mHeight;
        rawdata.resize(stride * w * h);
        auto bdata = (uint8_t const *)data;
        for (auto i = 0; i < w * h; ++i)
            std::copy(bdata, bdata + stride, rawdata.data() + i * stride);

        // upload
        bind().setData(getInternalFormat(), w, h, format, type, rawdata.data());

    }
    else
    {
        glClearTexImage(mObjectName, 0, format, type, data);
    }
#else
    error() << "TextureRectangle::clear is only supported for OpenGL 4.4+ " << to_string(this);
#endif
}

std::vector<char> TextureRectangle::BoundTextureRectangle::getData(GLenum format, GLenum type)
{
    if (!isCurrent())
        return {};
    checkValidGLOW();

    auto target = texture->mTarget;

    size_t dataSize = channelsOfFormat(format) * sizeOfTypeInBytes(type);

    GLint sWidth;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &sWidth);
    dataSize *= sWidth;

    GLint sHeight;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &sHeight);
    dataSize *= sHeight;

    std::vector<char> data(dataSize);
    getData(format, type, data.size(), data.data());
    return data;
}

void TextureRectangle::BoundTextureRectangle::getData(GLenum format, GLenum type, size_t bufferSize, void *buffer)
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
    glGetTexImage(texture->mTarget, 0, format, type, buffer);
}

SharedTextureData TextureRectangle::BoundTextureRectangle::getTextureData()
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
    auto target = texture->mTarget;
    {
        auto lvl = 0;
        GLint w;
        GLint h;
        GLint d;
        GLenum internalFormat;
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_DEPTH, &d);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_INTERNAL_FORMAT, (GLint *)&internalFormat);

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

void TextureRectangle::BoundTextureRectangle::writeToFile(const std::string &filename)
{
    getTextureData()->saveToFile(filename);
}


TextureRectangle::BoundTextureRectangle::BoundTextureRectangle (TextureRectangle *texture) : texture(texture)
{
    checkValidGLOW();
    glGetIntegerv(texture->mBindingTarget, &previousTexture);
    glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
    glBindTexture(texture->mTarget, texture->mObjectName);

    previousTexturePtr = sCurrentTexture;
    sCurrentTexture = this;
}

TextureRectangle::BoundTextureRectangle::BoundTextureRectangle (TextureRectangle::BoundTextureRectangle &&rhs)
  : texture(rhs.texture), previousTexture(rhs.previousTexture), previousTexturePtr(rhs.previousTexturePtr)
{
    // invalidate rhs
    rhs.previousTexture = -1;
}

TextureRectangle::BoundTextureRectangle::~BoundTextureRectangle ()
{
    if (previousTexture != -1) // if valid
    {
        checkValidGLOW();
        glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
        glBindTexture(texture->mTarget, previousTexture);
        sCurrentTexture = previousTexturePtr;
    }
}
