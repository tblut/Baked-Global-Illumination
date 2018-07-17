// This file is auto-generated and should not be modified directly.
#include "Texture2DMultisample.hh"

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
static GLOW_THREADLOCAL Texture2DMultisample::BoundTexture2DMultisample *sCurrentTexture = nullptr;

Texture2DMultisample::BoundTexture2DMultisample *Texture2DMultisample::getCurrentTexture()
{
    return sCurrentTexture;
}


GLenum Texture2DMultisample::getUniformType() const
{
    auto fmt = getInternalFormat();

    if (isSignedIntegerInternalFormat(fmt))
        return GL_INT_SAMPLER_2D_MULTISAMPLE;
    else if (isUnsignedIntegerInternalFormat(fmt))
        return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;
    else
        return GL_SAMPLER_2D_MULTISAMPLE;
}

Texture2DMultisample::Texture2DMultisample (GLenum internalFormat)
  : Texture(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BINDING_2D_MULTISAMPLE, internalFormat)
{
}

SharedTexture2DMultisample Texture2DMultisample::create(int width, int height, GLenum internalFormat)
{
    auto tex = std::make_shared<Texture2DMultisample>(internalFormat);
    tex->bind().resize(width, height);
    return tex;
}



bool Texture2DMultisample::BoundTexture2DMultisample::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentTexture == this, "Currently bound FBO does NOT match represented Texture " << to_string(texture), return false);
    return true;
}


void Texture2DMultisample::BoundTexture2DMultisample::setMinFilter(GLenum filter)
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
        warning() << "Texture2DMultisample does not support MipMapping. " << to_string(texture);

    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, filter);
    texture->mMinFilter = filter;
}

void Texture2DMultisample::BoundTexture2DMultisample::setMagFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, filter);
    texture->mMagFilter = filter;
}

void Texture2DMultisample::BoundTexture2DMultisample::setFilter(GLenum magFilter, GLenum minFilter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, magFilter);
    texture->mMinFilter = minFilter;
    texture->mMagFilter = magFilter;
}

void Texture2DMultisample::BoundTexture2DMultisample::setAnisotropicFiltering(GLfloat samples)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    samples = glm::clamp(samples, 1.f, limits::maxAnisotropy);
    glTexParameterf(texture->mTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, samples);
    texture->mAnisotropicFiltering = samples;
}

void Texture2DMultisample::BoundTexture2DMultisample::setBorderColor(glm::vec4 const& color)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameterfv(texture->mTarget, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    texture->mBorderColor = color;
}

void Texture2DMultisample::BoundTexture2DMultisample::setWrapS(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrap);
    texture->mWrapS = wrap;
}
void Texture2DMultisample::BoundTexture2DMultisample::setWrapT(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrap);
    texture->mWrapT = wrap;
}

void Texture2DMultisample::BoundTexture2DMultisample::setWrap(GLenum wrapS, GLenum wrapT)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrapS);
    texture->mWrapS = wrapS;
    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrapT);
    texture->mWrapT = wrapT;
}

void Texture2DMultisample::BoundTexture2DMultisample::setCompareMode(GLenum mode)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_COMPARE_MODE, mode);
    texture->mCompareMode = mode;
}

void Texture2DMultisample::BoundTexture2DMultisample::setCompareFunc(GLenum func)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_COMPARE_FUNC, func);
    texture->mCompareFunc = func;
}

void Texture2DMultisample::BoundTexture2DMultisample::setDepthStencilMode(GLenum mode)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_DEPTH_STENCIL_TEXTURE_MODE, mode);
    texture->mDepthStencilMode = mode;
}


void Texture2DMultisample::BoundTexture2DMultisample::resize(int width, int height)
{
    if (!isCurrent())
        return;
    checkValidGLOW();


    texture->mWidth = width;
    texture->mHeight = height;


    auto const samples = 4; // TODO: configurable
    auto const fixedSampleLocs = GL_FALSE; // TODO: configurable
    glTexImage2DMultisample(texture->mTarget, samples, texture->mInternalFormat, width, height, fixedSampleLocs);

}


Texture2DMultisample::BoundTexture2DMultisample::BoundTexture2DMultisample (Texture2DMultisample *texture) : texture(texture)
{
    checkValidGLOW();
    glGetIntegerv(texture->mBindingTarget, &previousTexture);
    glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
    glBindTexture(texture->mTarget, texture->mObjectName);

    previousTexturePtr = sCurrentTexture;
    sCurrentTexture = this;
}

Texture2DMultisample::BoundTexture2DMultisample::BoundTexture2DMultisample (Texture2DMultisample::BoundTexture2DMultisample &&rhs)
  : texture(rhs.texture), previousTexture(rhs.previousTexture), previousTexturePtr(rhs.previousTexturePtr)
{
    // invalidate rhs
    rhs.previousTexture = -1;
}

Texture2DMultisample::BoundTexture2DMultisample::~BoundTexture2DMultisample ()
{
    if (previousTexture != -1) // if valid
    {
        checkValidGLOW();
        glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
        glBindTexture(texture->mTarget, previousTexture);
        sCurrentTexture = previousTexturePtr;
    }
}
