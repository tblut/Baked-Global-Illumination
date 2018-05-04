// This file is auto-generated and should not be modified directly.
#include "TextureBuffer.hh"

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
static GLOW_THREADLOCAL TextureBuffer::BoundTextureBuffer *sCurrentTexture = nullptr;

TextureBuffer::BoundTextureBuffer *TextureBuffer::getCurrentTexture()
{
    return sCurrentTexture;
}


GLenum TextureBuffer::getUniformType() const
{
    auto fmt = getInternalFormat();

    if (isSignedIntegerInternalFormat(fmt))
        return GL_INT_SAMPLER_BUFFER;
    else if (isUnsignedIntegerInternalFormat(fmt))
        return GL_UNSIGNED_INT_SAMPLER_BUFFER;
    else
        return GL_SAMPLER_BUFFER;
}

TextureBuffer::TextureBuffer (GLenum internalFormat)
  : Texture(GL_TEXTURE_BUFFER, GL_TEXTURE_BINDING_BUFFER, internalFormat)
{
}

SharedTextureBuffer TextureBuffer::create(int size, GLenum internalFormat)
{
    auto tex = std::make_shared<TextureBuffer>(internalFormat);
    tex->bind().resize(size);
    return tex;
}


SharedTextureBuffer TextureBuffer::createFromFile(const std::string &filename, ColorSpace colorSpace)
{
    auto t = createFromData(TextureData::createFromFile(filename, colorSpace));
    t->setObjectLabel(filename);
    return t;
}

SharedTextureBuffer TextureBuffer::createFromFile(const std::string &filename, GLenum internalFormat, ColorSpace colorSpace)
{
    auto t = createFromData(TextureData::createFromFile(filename, colorSpace), internalFormat);
    t->setObjectLabel(filename);
    return t;
}

SharedTextureBuffer TextureBuffer::createFromData(const SharedTextureData &data)
{
    if (!data)
    {
        error() << "TextureBuffer::createFromData failed, no data provided";
        return nullptr;
    }

    if (data->getPreferredInternalFormat() == GL_INVALID_ENUM)
    {
        error() << "TextureBuffer::createFromData failed, no preferred internal format specified";
        return nullptr;
    }

    auto tex = std::make_shared<TextureBuffer>(data->getPreferredInternalFormat());
    tex->bind().setData(data->getPreferredInternalFormat(), data);
    return tex;
}

SharedTextureBuffer TextureBuffer::createFromData(const SharedTextureData &data, GLenum internalFormat)
{
    if (!data)
    {
        error() << "TextureBuffer::createFromData failed, no data provided";
        return nullptr;
    }

    auto tex = std::make_shared<TextureBuffer>(internalFormat);
    tex->bind().setData(internalFormat, data);
    return tex;
}

bool TextureBuffer::BoundTextureBuffer::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentTexture == this, "Currently bound FBO does NOT match represented Texture " << to_string(texture), return false);
    return true;
}




void TextureBuffer::BoundTextureBuffer::resize(int size)
{
    if (!isCurrent())
        return;
    checkValidGLOW();


    texture->mSize = size;


    warning() << "not implemented. " << to_string(texture);

}

void TextureBuffer::BoundTextureBuffer::setData(GLenum internalFormat, int size, GLenum format, GLenum type, const GLvoid *data)
{
    if (!isCurrent())
        return;
    checkValidGLOW();


    texture->mSize = size;
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
        glTexSubImage1D(texture->mTarget, 0, 0, size, format, type, data);
    else
        glTexImage1D(texture->mTarget, 0, texture->mInternalFormat, size, 0, format, type, data);
}

void TextureBuffer::BoundTextureBuffer::setSubData(int offset, int size, GLenum format, GLenum type, const GLvoid *data)
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

    glTexSubImage1D(texture->mTarget, 0, offset, size, format, type, data);
 }

void TextureBuffer::BoundTextureBuffer::setData(GLenum internalFormat, const SharedTextureData &data)
{
    if (!isCurrent())
        return;

    texture->mInternalFormat = internalFormat; // format first, then resize
    resize(data->getWidth());

    // set all level 0 surfaces
    for (auto const &surf : data->getSurfaces())
        if (surf->getMipmapLevel() == 0)
            setSubData(surf->getOffsetX(),
                       surf->getWidth(),
                       surf->getFormat(), surf->getType(),
                       surf->getData().data());


}


std::vector<char> TextureBuffer::BoundTextureBuffer::getData(GLenum format, GLenum type)
{
    if (!isCurrent())
        return {};
    checkValidGLOW();

    auto target = texture->mTarget;

    size_t dataSize = channelsOfFormat(format) * sizeOfTypeInBytes(type);

    GLint sWidth;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &sWidth);
    dataSize *= sWidth;

    std::vector<char> data(dataSize);
    getData(format, type, data.size(), data.data());
    return data;
}

void TextureBuffer::BoundTextureBuffer::getData(GLenum format, GLenum type, size_t bufferSize, void *buffer)
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

SharedTextureData TextureBuffer::BoundTextureBuffer::getTextureData()
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

void TextureBuffer::BoundTextureBuffer::writeToFile(const std::string &filename)
{
    getTextureData()->saveToFile(filename);
}


TextureBuffer::BoundTextureBuffer::BoundTextureBuffer (TextureBuffer *texture) : texture(texture)
{
    checkValidGLOW();
    glGetIntegerv(texture->mBindingTarget, &previousTexture);
    glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
    glBindTexture(texture->mTarget, texture->mObjectName);

    previousTexturePtr = sCurrentTexture;
    sCurrentTexture = this;
}

TextureBuffer::BoundTextureBuffer::BoundTextureBuffer (TextureBuffer::BoundTextureBuffer &&rhs)
  : texture(rhs.texture), previousTexture(rhs.previousTexture), previousTexturePtr(rhs.previousTexturePtr)
{
    // invalidate rhs
    rhs.previousTexture = -1;
}

TextureBuffer::BoundTextureBuffer::~BoundTextureBuffer ()
{
    if (previousTexture != -1) // if valid
    {
        checkValidGLOW();
        glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
        glBindTexture(texture->mTarget, previousTexture);
        sCurrentTexture = previousTexturePtr;
    }
}
