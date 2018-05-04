#include "Buffer.hh"

#include "glow/glow.hh"

#include "glow/common/ogl_typeinfo.hh"

#include <cassert>
#include <limits>

using namespace glow;

Buffer::Buffer(GLenum bufferType, const SharedBuffer &originalBuffer)
  : mType(bufferType), mOriginalBuffer(originalBuffer)
{
    checkValidGLOW();

    if (isAliasedBuffer())
    {
        if (mOriginalBuffer->isAliasedBuffer()) // follow alias chain
        {
            mOriginalBuffer = mOriginalBuffer->getOriginalBuffer();
            assert(!mOriginalBuffer->isAliasedBuffer());
        }

        mObjectName = mOriginalBuffer->getObjectName();
    }
    else
    {
        mObjectName = std::numeric_limits<decltype(mObjectName)>::max();
        glGenBuffers(1, &mObjectName);
        assert(mObjectName != std::numeric_limits<decltype(mObjectName)>::max() && "No OpenGL Context?");

        // bind the buffer once to guarantee that object is valid
        GLint prevBuffer = 0;
        glGetIntegerv(bufferBindingOf(bufferType), &prevBuffer);
        glBindBuffer(bufferType, mObjectName);
        // buffer is now valid
        glBindBuffer(bufferType, prevBuffer);
    }
}

Buffer::~Buffer()
{
    if (!isAliasedBuffer())
    {
        checkValidGLOW();

        glDeleteBuffers(1, &mObjectName);
    }
}
