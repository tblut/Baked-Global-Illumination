#include "ShaderStorageBuffer.hh"

#include "glow/glow.hh"

#include <glow/common/runtime_assert.hh>
#include <glow/common/thread_local.hh>

using namespace glow;

/// Currently bound buffer
static GLOW_THREADLOCAL ShaderStorageBuffer::BoundShaderStorageBuffer *sCurrentBuffer = nullptr;

ShaderStorageBuffer::BoundShaderStorageBuffer *ShaderStorageBuffer::getCurrentBuffer()
{
    return sCurrentBuffer;
}

ShaderStorageBuffer::ShaderStorageBuffer(const SharedBuffer &originalBuffer)
  : Buffer(GL_SHADER_STORAGE_BUFFER, originalBuffer)
{
}

SharedShaderStorageBuffer ShaderStorageBuffer::create()
{
    return std::make_shared<ShaderStorageBuffer>();
}

SharedShaderStorageBuffer ShaderStorageBuffer::createAliased(const SharedBuffer &originalBuffer)
{
    return std::make_shared<ShaderStorageBuffer>(originalBuffer);
}

bool ShaderStorageBuffer::BoundShaderStorageBuffer::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentBuffer == this, "Currently bound UB does NOT match represented buffer " << to_string(buffer), return false);
    return true;
}

void ShaderStorageBuffer::BoundShaderStorageBuffer::setData(size_t size, const void *data, GLenum usage)
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
}

void ShaderStorageBuffer::BoundShaderStorageBuffer::getData(void *destination, size_t maxSize, bool warnOnTruncate)
{
    auto size = getSize();
    if (maxSize > 0 && maxSize < size)
    {
        if (warnOnTruncate)
            warning() << "Buffer size is " << size << " B but only " << maxSize << " B is guaranteed. " << to_string(buffer);

        size = maxSize;
    }

    checkValidGLOW();
    glGetBufferSubData(buffer->getType(), 0, size, destination);
}

size_t ShaderStorageBuffer::BoundShaderStorageBuffer::getSize() const
{
    if (!isCurrent())
        return 0u;

    checkValidGLOW();
    int bufSize = 0;
    glGetBufferParameteriv(buffer->getType(), GL_BUFFER_SIZE, &bufSize);

    return bufSize;
}

void ShaderStorageBuffer::BoundShaderStorageBuffer::reserve(size_t sizeInBytes, GLenum usage)
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, nullptr, usage);
}

ShaderStorageBuffer::BoundShaderStorageBuffer::BoundShaderStorageBuffer(ShaderStorageBuffer *buffer) : buffer(buffer)
{
    checkValidGLOW();
    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &previousBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->getObjectName());

    previousBufferPtr = sCurrentBuffer;
    sCurrentBuffer = this;
}

ShaderStorageBuffer::BoundShaderStorageBuffer::BoundShaderStorageBuffer(ShaderStorageBuffer::BoundShaderStorageBuffer &&rhs)
  : buffer(rhs.buffer), previousBuffer(rhs.previousBuffer), previousBufferPtr(rhs.previousBufferPtr)
{
    // invalidate rhs
    rhs.previousBuffer = -1;
}

ShaderStorageBuffer::BoundShaderStorageBuffer::~BoundShaderStorageBuffer()
{
    if (previousBuffer != -1) // if valid
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, previousBuffer);
        sCurrentBuffer = previousBufferPtr;
    }
}
