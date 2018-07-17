#include "AtomicCounterBuffer.hh"

#include "glow/glow.hh"

#include <glow/common/runtime_assert.hh>
#include <glow/common/thread_local.hh>

using namespace glow;

/// Currently bound buffer
static GLOW_THREADLOCAL AtomicCounterBuffer::BoundAtomicCounterBuffer *sCurrentBuffer = nullptr;

AtomicCounterBuffer::BoundAtomicCounterBuffer *AtomicCounterBuffer::getCurrentBuffer()
{
    return sCurrentBuffer;
}

AtomicCounterBuffer::AtomicCounterBuffer() : Buffer(GL_SHADER_STORAGE_BUFFER) {}

SharedAtomicCounterBuffer AtomicCounterBuffer::create(uint32_t value, int count)
{
    auto b = std::make_shared<AtomicCounterBuffer>();
    b->bind().setCounters(value, count);
    return b;
}

SharedAtomicCounterBuffer AtomicCounterBuffer::create(const std::vector<uint32_t> &values)
{
    auto b = std::make_shared<AtomicCounterBuffer>();
    b->bind().setCounters(values);
    return b;
}

bool AtomicCounterBuffer::BoundAtomicCounterBuffer::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentBuffer == this, "Currently bound UB does NOT match represented buffer " << to_string(buffer), return false);
    return true;
}

void AtomicCounterBuffer::BoundAtomicCounterBuffer::setCounters(const std::vector<uint32_t> &values, GLenum usage)
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    glBufferData(buffer->getType(), values.size() * sizeof(uint32_t), values.data(), usage);
}

void AtomicCounterBuffer::BoundAtomicCounterBuffer::setCounters(uint32_t value, int count, GLenum usage)
{
    setCounters(std::vector<uint32_t>(count, value));
}

uint32_t AtomicCounterBuffer::BoundAtomicCounterBuffer::getCounter(int pos)
{
    if (!isCurrent())
        return 0u;

    checkValidGLOW();
    uint32_t value;
    glGetBufferSubData(buffer->getType(), pos * sizeof(uint32_t), sizeof(uint32_t), &value);
    return value;
}

std::vector<uint32_t> AtomicCounterBuffer::BoundAtomicCounterBuffer::getCounters()
{
    if (!isCurrent())
        return {};

    checkValidGLOW();
    auto cnt = getCounterSize();
    std::vector<uint32_t> counters(cnt);
    glGetBufferSubData(buffer->getType(), 0, cnt * sizeof(uint32_t), counters.data());
    return counters;
}

size_t AtomicCounterBuffer::BoundAtomicCounterBuffer::getByteSize() const
{
    if (!isCurrent())
        return 0u;

    checkValidGLOW();
    int bufSize = 0;
    glGetBufferParameteriv(buffer->getType(), GL_BUFFER_SIZE, &bufSize);

    return bufSize;
}

int AtomicCounterBuffer::BoundAtomicCounterBuffer::getCounterSize() const
{
    return getByteSize() / sizeof(uint32_t);
}

AtomicCounterBuffer::BoundAtomicCounterBuffer::BoundAtomicCounterBuffer(AtomicCounterBuffer *buffer) : buffer(buffer)
{
    checkValidGLOW();
    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &previousBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->getObjectName());

    previousBufferPtr = sCurrentBuffer;
    sCurrentBuffer = this;
}

AtomicCounterBuffer::BoundAtomicCounterBuffer::BoundAtomicCounterBuffer(AtomicCounterBuffer::BoundAtomicCounterBuffer &&rhs)
  : buffer(rhs.buffer), previousBuffer(rhs.previousBuffer), previousBufferPtr(rhs.previousBufferPtr)
{
    // invalidate rhs
    rhs.previousBuffer = -1;
}

AtomicCounterBuffer::BoundAtomicCounterBuffer::~BoundAtomicCounterBuffer()
{
    if (previousBuffer != -1) // if valid
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, previousBuffer);
        sCurrentBuffer = previousBufferPtr;
    }
}
