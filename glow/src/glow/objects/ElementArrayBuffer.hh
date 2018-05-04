#pragma once

#include "Buffer.hh"

#include "glow/common/warn_unused.hh"

#include <vector>

namespace glow
{
GLOW_SHARED(class, ElementArrayBuffer);

class ElementArrayBuffer final : public Buffer
{
    GLOW_NON_COPYABLE(ElementArrayBuffer);

public:
    struct BoundElementArrayBuffer;

private:
    /// Type of the index data.
    /// Supported:
    ///   * GL_UNSIGNED_BYTE
    ///   * GL_UNSIGNED_SHORT
    ///   * GL_UNSIGNED_INT
    GLenum mIndexType = GL_INVALID_ENUM;

    /// Number of indices
    int mIndexCount = 0;

public: // getter
    GLenum getIndexType() const { return mIndexType; }
    int getIndexCount() const { return mIndexCount; }
    /// Gets the currently bound EAB (nullptr if none)
    static BoundElementArrayBuffer* getCurrentBuffer();

public:
    /// RAII-object that defines a "bind"-scope for an ElementArrayBuffer
    /// All functions that operate on the currently bound buffer are accessed here
    struct BoundElementArrayBuffer
    {
        GLOW_RAII_CLASS(BoundElementArrayBuffer);

        /// Backreference to the buffer
        ElementArrayBuffer* const buffer;

        /// Uploads a set of indices
        /// Automatically picks the right data type
        ///
        /// usage is a hint to the GL implementation as to how a buffer object's data store will be accessed.
        /// This enables the GL implementation to make more intelligent decisions that may significantly impact buffer
        /// object performance. It does not, however, constrain the actual usage of the data store.
        /// (see https://www.opengl.org/sdk/docs/man/html/glBufferData.xhtml)
        void setIndices(std::vector<int8_t> const& indices, GLenum usage = GL_STATIC_DRAW);
        void setIndices(std::vector<uint8_t> const& indices, GLenum usage = GL_STATIC_DRAW);
        void setIndices(std::vector<int16_t> const& indices, GLenum usage = GL_STATIC_DRAW);
        void setIndices(std::vector<uint16_t> const& indices, GLenum usage = GL_STATIC_DRAW);
        void setIndices(std::vector<int32_t> const& indices, GLenum usage = GL_STATIC_DRAW);
        void setIndices(std::vector<uint32_t> const& indices, GLenum usage = GL_STATIC_DRAW);
        void setIndices(int indexCount, const int8_t* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        void setIndices(int indexCount, const uint8_t* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        void setIndices(int indexCount, const int16_t* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        void setIndices(int indexCount, const uint16_t* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        void setIndices(int indexCount, const int32_t* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        void setIndices(int indexCount, const uint32_t* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        void setIndices(int indexCount, GLenum indexType, const void* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        template <typename IndexT, size_t N>
        void setIndices(const IndexT(&data)[N], GLenum usage = GL_STATIC_DRAW)
        {
            setIndices((int)N, data, usage);
        }

    private:
        GLint previousBuffer;                       ///< previously bound buffer
        BoundElementArrayBuffer* previousBufferPtr; ///< previously bound buffer
        BoundElementArrayBuffer(ElementArrayBuffer* buffer);
        friend class ElementArrayBuffer;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundElementArrayBuffer(BoundElementArrayBuffer&&); // allow move
        ~BoundElementArrayBuffer();
    };

public:
    ElementArrayBuffer();

    /// Binds this vertex array.
    /// Unbinding is done when the returned object runs out of scope.
    /// CAUTION: Cannot be used while a VAO is bound! (runtime error)
    GLOW_WARN_UNUSED BoundElementArrayBuffer bind() { return {this}; }
public: // static construction
    /// Creates an empty element array buffer
    /// Same as std::make_shared<ElementArrayBuffer>();
    static SharedElementArrayBuffer create();

    /// Creates a element array buffer with the given indices
    /// Automatically picks the correct index type
    /// CAUTION: Cannot be used while a VAO is bound! (runtime error)
    static SharedElementArrayBuffer create(std::vector<int8_t> const& indices);
    static SharedElementArrayBuffer create(std::vector<uint8_t> const& indices);
    static SharedElementArrayBuffer create(std::vector<int16_t> const& indices);
    static SharedElementArrayBuffer create(std::vector<uint16_t> const& indices);
    static SharedElementArrayBuffer create(std::vector<int32_t> const& indices);
    static SharedElementArrayBuffer create(std::vector<uint32_t> const& indices);
    static SharedElementArrayBuffer create(int indexCount, const int8_t* data);
    static SharedElementArrayBuffer create(int indexCount, const uint8_t* data);
    static SharedElementArrayBuffer create(int indexCount, const int16_t* data);
    static SharedElementArrayBuffer create(int indexCount, const uint16_t* data);
    static SharedElementArrayBuffer create(int indexCount, const int32_t* data);
    static SharedElementArrayBuffer create(int indexCount, const uint32_t* data);
    template<typename T, std::size_t N>
    static SharedElementArrayBuffer create(const T (&data)[N])
    {
        return create(N, data);
    }

    /// Creates a element array buffer with the given indices
    /// Type must be specified explicitly
    /// CAUTION: Cannot be used while a VAO is bound! (runtime error)
    static SharedElementArrayBuffer create(int indexCount, GLenum indexType, const void* data = nullptr);
};
}
