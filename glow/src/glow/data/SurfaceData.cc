#include "SurfaceData.hh"
#include "TextureData.hh"

#include <glow/common/ogl_typeinfo.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/objects/Texture2DArray.hh>
#include <glow/objects/Framebuffer.hh>

#include <algorithm>
#include <cassert>

using namespace glow;

SurfaceData::SurfaceData()
{
}

SharedSurfaceData SurfaceData::getSubData(int x, int y, int z, int width, int height, int depth) const
{
    assert(x < mWidth && x + width <= mWidth);
    assert(y < mHeight && y + height <= mHeight);
    assert(z < mDepth && z + depth <= mDepth);

    auto s = std::make_shared<SurfaceData>();

    s->setWidth(width);
    s->setHeight(height);
    s->setDepth(depth);

    s->setOffsetX(mOffsetX + x);
    s->setOffsetX(mOffsetY + y);
    s->setOffsetX(mOffsetZ + z);

    s->setTarget(mTarget);
    s->setType(mType);
    s->setFormat(mFormat);

    assert(mData.size() % (mWidth * mHeight * mDepth) == 0 && "Inconsistent data");
    auto bytesPerPixel = mData.size() / (mWidth * mHeight * mDepth);
    std::vector<char> newData;
    newData.resize(bytesPerPixel * width * height * depth);
    for (auto dz = 0; dz < depth; ++dz)
        for (auto dy = 0; dy < height; ++dy)
        {
            auto src = mData.data() + (((z + dz) * mHeight + (y + dy)) * mWidth + x) * bytesPerPixel;
            auto dest = newData.data() + (dz * height + dy) * width * bytesPerPixel;
            std::copy(src, src + bytesPerPixel * width, dest);
        }

    s->setData(newData);

    return s;
}

SharedTextureData SurfaceData::makeTextureData() const
{
    auto s = std::make_shared<SurfaceData>();

    s->setWidth(mWidth);
    s->setHeight(mHeight);
    s->setDepth(mDepth);
    s->setData(mData);

    s->setTarget(mTarget);
    s->setType(mType);
    s->setFormat(mFormat);

    auto tex = std::make_shared<TextureData>();
    tex->setWidth(mWidth);
    tex->setHeight(mHeight);
    tex->setDepth(mDepth);

    tex->addSurface(s);

    return tex;
}

SharedSurfaceData SurfaceData::convertTo(GLenum format, GLenum type) const
{
    auto internalFormat = GL_RGBA32F; // TODO: integer, depth-stencil?
    auto tmpTex = Texture2DArray::createStorageImmutable(mWidth, mHeight, mDepth, internalFormat, 1);


    auto t = tmpTex->bind();
    t.setData(internalFormat, mWidth, mHeight, mDepth, mFormat, mType, mData.data());
    // maybe required in the future.
    /*{ // clear 0001
        auto fb = Framebuffer::create({{"A", tmpTex}});
        auto f = fb->bind();
        GLOW_SCOPED(clearColor, 0, 0, 0, 1);
        bool r = false, g = false, b = false, a = false;
        switch (format)
        {
        case GL_STENCIL_INDEX:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
            warning() << "Not implemented";
            break;

        case GL_RED:
        case GL_RED_INTEGER:
            g = true;
            b = true;
            a = true;
            break;

        case GL_RG:
        case GL_RG_INTEGER:
            b = true;
            a = true;
            break;

        case GL_RGB:
        case GL_RGB_INTEGER:
        case GL_BGR:
        case GL_BGR_INTEGER:
            a = true;
            break;

        case GL_RGBA:
        case GL_RGBA_INTEGER:
        case GL_BGRA:
        case GL_BGRA_INTEGER:
            break;

        default:
            error() << "Invalid format " << format;
            break;
        }
        GLOW_SCOPED(colorMask, r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);
    }*/
    auto newData = t.getData(format, type);

    auto s = std::make_shared<SurfaceData>();

    s->setOffsetX(mOffsetX);
    s->setOffsetY(mOffsetY);
    s->setOffsetZ(mOffsetZ);

    s->setWidth(mWidth);
    s->setHeight(mHeight);
    s->setDepth(mDepth);
    s->setData(newData);

    s->setMipmapLevel(mMipmapLevel);

    s->setTarget(mTarget);
    s->setType(type);
    s->setFormat(format);

    return s;
}
