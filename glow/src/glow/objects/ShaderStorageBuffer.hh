#pragma once

#include <glow/common/shared.hh>
#include <glow/common/warn_unused.hh>
#include <glow/common/log.hh>

#include "Buffer.hh"

#include <vector>
#include <string>
#include <map>

namespace glow
{
GLOW_SHARED(class, ShaderStorageBuffer);
/**
 * A shader buffer for generic data
 *
 * Usage:
 *   program->setShaderStorageBuffer("bufferName", buffer);
 *
 * See std140.hh for ways to safely use C++ structs
 *
 */
class ShaderStorageBuffer : public Buffer
{
public:
    struct BoundShaderStorageBuffer;

public: // getter
    /// Gets the currently bound ShaderStorageBuffer (nullptr if none)
    static BoundShaderStorageBuffer* getCurrentBuffer();

public:
    /// RAII-object that defines a "bind"-scope for an ShaderStorageBuffer
    /// All functions that operate on the currently bound buffer are accessed here
    struct BoundShaderStorageBuffer
    {
        GLOW_RAII_CLASS(BoundShaderStorageBuffer);

        /// Backreference to the buffer
        ShaderStorageBuffer* const buffer;

        /// Sets the data of this uniform buffer (generic version)
        void setData(size_t size, const void* data, GLenum usage = GL_STATIC_DRAW);
        /// Sets the data of this uniform buffer (vector-of-data version)
        template <typename DataT>
        void setData(std::vector<DataT> const& data, GLenum usage = GL_STATIC_DRAW)
        {
            setData(data.size() * sizeof(DataT), data.data(), usage);
        }
        /// Sets the data of this uniform buffer (POD version)
        template <typename DataT>
        void setData(DataT const& data, GLenum usage = GL_STATIC_DRAW)
        {
            setData(sizeof(data), &data, usage);
        }

        /// Writes all buffer data into the given memory
        /// Data is truncated to maxSize
        void getData(void* destination, size_t maxSize = 0, bool warnOnTruncate = true);

        /// Reads all data into a vector
        /// Generates an error if (size % sizeof(DataT)) != 0
        /// Optional: if maxCount is bigger than zero, it limits the number of returned elements
        template <typename DataT>
        std::vector<DataT> getData(size_t maxCount = 0)
        {
            auto size = getSize();
            if ((size % sizeof(DataT)) != 0)
            {
                error() << "Buffer size is not multiple of data type! " << to_string(buffer);
                return {};
            }
            auto count = size / sizeof(DataT);
            if (maxCount > 0 && maxCount < count)
                count = maxCount;

            std::vector<DataT> data(count);
            getData(data.data(), count * sizeof(DataT), false); // truncating is behavior-by-design
            return data;
        }

        /// Returns the size in bytes of this buffer
        size_t getSize() const;

        /// Reserves a certain buffer size
        /// CAUTION: will probably invalidate all data
        void reserve(size_t sizeInBytes, GLenum usage = GL_STATIC_DRAW);

    private:
        GLint previousBuffer;                        ///< previously bound buffer
        BoundShaderStorageBuffer* previousBufferPtr; ///< previously bound buffer
        BoundShaderStorageBuffer(ShaderStorageBuffer* buffer);
        friend class ShaderStorageBuffer;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundShaderStorageBuffer(BoundShaderStorageBuffer&&); // allow move
        ~BoundShaderStorageBuffer();
    };

public:
    ShaderStorageBuffer(SharedBuffer const& originalBuffer = nullptr);

    /// Binds this uniform buffer.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundShaderStorageBuffer bind() { return {this}; }
public: // static construction
    /// Creates an empty shader storage buffer
    /// Same as std::make_shared<ShaderStorageBuffer>();
    static SharedShaderStorageBuffer create();

    /// Creates a shader storage buffer that shares memory with another buffer
    static SharedShaderStorageBuffer createAliased(SharedBuffer const& originalBuffer);
};
}
