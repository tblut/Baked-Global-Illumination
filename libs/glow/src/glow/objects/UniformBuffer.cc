#include "UniformBuffer.hh"

#include <glow/glow.hh>

#include <glow/common/runtime_assert.hh>
#include <glow/common/thread_local.hh>

using namespace glow;

/// Currently bound buffer
static GLOW_THREADLOCAL UniformBuffer::BoundUniformBuffer *sCurrentBuffer = nullptr;

UniformBuffer::BoundUniformBuffer *UniformBuffer::getCurrentBuffer()
{
    return sCurrentBuffer;
}

UniformBuffer::UniformBuffer() : Buffer(GL_UNIFORM_BUFFER)
{
}

void UniformBuffer::addVerification(int offset, const std::string &nameInShader)
{
    mVerificationOffsets[nameInShader] = offset;
}

SharedUniformBuffer UniformBuffer::create()
{
    return std::make_shared<UniformBuffer>();
}

bool UniformBuffer::BoundUniformBuffer::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentBuffer == this, "Currently bound UB does NOT match represented buffer " << to_string(buffer), return false);
    return true;
}

void UniformBuffer::BoundUniformBuffer::setData(size_t size, const void *data, GLenum usage)
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
}

UniformBuffer::BoundUniformBuffer::BoundUniformBuffer(UniformBuffer *buffer) : buffer(buffer)
{
    checkValidGLOW();
    glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &previousBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer->getObjectName());

    previousBufferPtr = sCurrentBuffer;
    sCurrentBuffer = this;
}

UniformBuffer::BoundUniformBuffer::BoundUniformBuffer(UniformBuffer::BoundUniformBuffer &&rhs)
  : buffer(rhs.buffer), previousBuffer(rhs.previousBuffer), previousBufferPtr(rhs.previousBufferPtr)
{
    // invalidate rhs
    rhs.previousBuffer = -1;
}

UniformBuffer::BoundUniformBuffer::~BoundUniformBuffer()
{
    if (previousBuffer != -1) // if valid
    {
        glBindBuffer(GL_UNIFORM_BUFFER, previousBuffer);
        sCurrentBuffer = previousBufferPtr;
    }
}
