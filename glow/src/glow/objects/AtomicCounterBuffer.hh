#pragma once

#include <glow/common/shared.hh>
#include <glow/common/warn_unused.hh>
#include <glow/common/log.hh>

#include "Buffer.hh"

#include <vector>
#include <string>

namespace glow
{
GLOW_SHARED(class, AtomicCounterBuffer);
/**
 * A buffer holding atomic counters
 *
 * Counters are always uint32_t, one buffer may hold multiple counters
 *
 * Usage in C++:
 *   program->setAtomicCounterBuffer(bindingPos, buffer);
 *
 * Usage in shader: (offset in byte)
 *      layout(binding = 0) uniform atomic_uint myArbitraryName1;
 *      layout(binding = 2, offset = 4) uniform atomic_uint myArbitraryName2;
 */
class AtomicCounterBuffer : public Buffer
{
public:
    struct BoundAtomicCounterBuffer;

public: // getter
    /// Gets the currently bound AtomicCounterBuffer (nullptr if none)
    static BoundAtomicCounterBuffer* getCurrentBuffer();

public:
    /// RAII-object that defines a "bind"-scope for an AtomicCounterBuffer
    /// All functions that operate on the currently bound buffer are accessed here
    struct BoundAtomicCounterBuffer
    {
        GLOW_RAII_CLASS(BoundAtomicCounterBuffer);

        /// Backreference to the buffer
        AtomicCounterBuffer* const buffer;

        /// Sets all counters to the given values (discards all previous data)
        void setCounters(std::vector<uint32_t> const& values, GLenum usage = GL_STATIC_DRAW);
        void setCounters(uint32_t value, int count = 1, GLenum usage = GL_STATIC_DRAW);

        /// Returns the value of a counter at the given position
        uint32_t getCounter(int pos = 0);
        /// Returns the value of all counters
        std::vector<uint32_t> getCounters();

        /// Returns the size in bytes of this buffer
        size_t getByteSize() const;
        /// Returns the number of counters in this buffer
        int getCounterSize() const;

    private:
        GLint previousBuffer;                        ///< previously bound buffer
        BoundAtomicCounterBuffer* previousBufferPtr; ///< previously bound buffer
        BoundAtomicCounterBuffer(AtomicCounterBuffer* buffer);
        friend class AtomicCounterBuffer;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundAtomicCounterBuffer(BoundAtomicCounterBuffer&&); // allow move
        ~BoundAtomicCounterBuffer();
    };

public:
    AtomicCounterBuffer();

    /// Binds this uniform buffer.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundAtomicCounterBuffer bind() { return {this}; }
public: // static construction
    /// Creates an atomic counter buffer with given initial value and count
    /// NOTE: default is a single counter with value 0
    static SharedAtomicCounterBuffer create(uint32_t value = 0, int count = 1);
    /// Creates an atomic counter buffer with given initial values
    static SharedAtomicCounterBuffer create(std::vector<uint32_t> const& values);
};
}
