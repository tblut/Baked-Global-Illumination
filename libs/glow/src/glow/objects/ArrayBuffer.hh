#pragma once

#include "Buffer.hh"

#include "ArrayBufferAttribute.hh"

#include "glow/common/gltypeinfo.hh"
#include "glow/common/log.hh"
#include "glow/common/warn_unused.hh"

#include <vector>

namespace glow
{
GLOW_SHARED(class, ArrayBuffer);

/**
 * An ArrayBuffer holds an array of per-vertex data. In its simplest form an
 * array of one attribute, for example the vertex position or texture-coordinate.
 * An ArrayBuffer however can also hold multiple attributes in an interleaved way.
 *
 * An ArrayBuffer can be drawn directly or indexed in combination with an ArrayBuffer.
 *
 * The combination of (multiple) attributes of (multiple) ArrayBuffers
 * and one (optional) ArrayBuffer is a VertexBufferObject or VertexArray.
 *
 * Note: In some documents ArrayBuffers (and sometimes ArrayBuffers) are
 *       called VertexBufferObjects, VBOs. The original extension that introduced
 *       these two new buffer types was called ARB_vertex_buffer_object but the buffers
 *       itself are called ArrayBuffer and ElementArrayBuffer.
 *
 ***************************************************************************************************************
 * Attributes:
 *************
 *
 * _type is the GL type
 * _size the number of elements in this attribute (1..4)
 * _normalized is the attribute normalization for int types
 *
 * Want to add tightly packed attributes in order?
 *  -> use defineAttribute()
 *
 * Want to add attributes with individual padding in order?
 *  -> use defineAttributeWithPadding()
 *
 * Want to add attributes out-of-order?
 *  -> use defineAttributeWithOffset()
 *
 * The stride size gets always set to the minimal stride size that covers all defined attributes (/w padding).
 * All define methods can get mixed!
 *
 * ab->defineAttribute(            "pos",       GL_FLOAT, 3    ); // stride: 12 bytes
 * ab->defineAttributeWithPadding( "color",     GL_CHAR,  3, 1 ); // stride: 12 + 3 + 1 = 16 bytes
 * ab->defineAttributeWithOffset(  "colorNorm", GL_CHAR,  3, 12, GL_TRUE ); // stride is still 16 as 12+3 <= 16!
 *
 * Convenience: most basic C++ and glm types can be used as template argument
 * (Automatically uses an appropriate AttributeMode! Float for floats, Integer for ints, Double for doubles,
 *  NormalizedInteger is not supported)
 *
 * ab->defineAttribute<glm::vec3>( "aPosition" );
 *
 * Convenience #2: (Automatically uses an appropriate AttributeMode! Float for floats, Integer for ints, Double for
 *                  doubles, NormalizedInteger is not supported)
 *
 * struct MyVertex {
 *  glm::vec3 aPosition;
 *  glm::vec2 texCoord;
 * };
 *
 * ab->defineAttribute(&MyVertex::aPosition, "aPosition");
 * ab->defineAttributes({{&MyVertex::aPosition, "aPosition"},
 *                       {&MyVertex::texCoord, "aTexCoord"}});
 *
 **************************************************************************************************************/
class ArrayBuffer final : public Buffer
{
    GLOW_NON_COPYABLE(ArrayBuffer);

public:
    struct BoundArrayBuffer;

private: // member
    /// Buffer stride in bytes
    GLuint mStride = 0;

    /// Number of elements
    GLuint mElementCount = 0;

    /// Attributes
    std::vector<ArrayBufferAttribute> mAttributes;

public: // getter
    /// Gets the currently bound AB (nullptr if none)
    static BoundArrayBuffer* getCurrentBuffer();

    GLuint getStride() const { return mStride; }
    GLuint getElementCount() const { return mElementCount; }
    std::vector<ArrayBufferAttribute> const& getAttributes() const { return mAttributes; }
    void setStride(GLuint stride) { mStride = stride; }

public:
    /// RAII-object that defines a "bind"-scope for an ArrayBuffer
    /// All functions that operate on the currently bound buffer are accessed here
    struct BoundArrayBuffer
    {
        GLOW_RAII_CLASS(BoundArrayBuffer);

        /// Backreference to the buffer
        ArrayBuffer* const buffer;

        /// Sets the raw data contained in the array buffer
        void setData(size_t sizeInBytes, const void* data = nullptr, GLenum usage = GL_STATIC_DRAW);
        /// Sets the array buffer data using a vector of POD (plain-old-data)
        /// Works excellent for glm types or user-defined vertex structs
        /// (e.g. struct Vertex { glm::vec3 pos; float f; })
        /// Warns if stride is not equal data size
        template <typename DataT>
        void setData(std::vector<DataT> const& data, GLenum usage = GL_STATIC_DRAW)
        {
            if (buffer->mStride > 0 && sizeof(DataT) != buffer->mStride)
                warning() << "Stride mismatch: expected " << buffer->mStride << ", got " << sizeof(DataT) << " "
                          << to_string(buffer);
            setData(sizeof(DataT) * data.size(), data.data(), usage);
        }
        /// Same as above
        /// Usage:
        ///   Vertex vertices[] = { ... }
        ///   setData(vertices);
        template <typename DataT, std::size_t N>
        void setData(const DataT (&data)[N], GLenum usage = GL_STATIC_DRAW)
        {
            if (buffer->mStride > 0 && sizeof(DataT) != buffer->mStride)
                warning() << "Stride mismatch: expected " << buffer->mStride << ", got " << sizeof(DataT) << " "
                          << to_string(buffer);
            setData(sizeof(DataT) * N, data, usage);
        }

    private:
        GLint previousBuffer;                ///< previously bound buffer
        BoundArrayBuffer* previousBufferPtr; ///< previously bound buffer
        BoundArrayBuffer(ArrayBuffer* buffer);
        friend class ArrayBuffer;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundArrayBuffer(BoundArrayBuffer&&); // allow move
        ~BoundArrayBuffer();
    };

public: // unbound functions
    /// Adds a fully specified attribute
    void defineAttribute(ArrayBufferAttribute const& a);

    /// Adds a set of fully specified attributes
    void defineAttributes(std::vector<ArrayBufferAttribute> const& attrs);

    /// Adds the attribute at the end of the existing attributes, stride gets computed automatically
    void defineAttribute(std::string const& name, GLenum type, GLint size, AttributeMode mode = AttributeMode::Float, GLuint divisor = 0);

    /// Adds the attribute at the end of the existing attributes, stride gets computed automatically
    /// + extra padding in bytes at the end
    void defineAttributeWithPadding(std::string const& name,
                                    GLenum type,
                                    GLint size,
                                    GLuint padding,
                                    AttributeMode mode = AttributeMode::Float,
                                    GLuint divisor = 0);

    /// Adds an attribute defined by an offset: this way an attribute can get added at arbitrary
    /// locations in the stride. If it's added at the end, the stride gets resized.
    /// This way attributes can even overlap, hope you know what you're doing...
    void defineAttributeWithOffset(std::string const& name,
                                   GLenum type,
                                   GLint size,
                                   GLuint offset,
                                   AttributeMode mode = AttributeMode::Float,
                                   GLuint divisor = 0);

    /// Attribute definition via template
    /// e.g. defineAttribute<glm::vec3>("aPosition");
    template <typename DataT>
    void defineAttribute(std::string const& name)
    {
        defineAttribute(name, glTypeOf<DataT>::type, glTypeOf<DataT>::size,
                        ArrayBufferAttribute::naturalModeOf(glTypeOf<DataT>::type), 0);
    }
    template <typename DataT>
    void defineAttribute(std::string const& name, AttributeMode mode, GLuint divisor = 0)
    {
        defineAttribute(name, glTypeOf<DataT>::type, glTypeOf<DataT>::size, mode, divisor);
    }
    /// Attribute definition with padding via template
    template <typename DataT>
    void defineAttributeWithPadding(std::string const& name, GLuint padding)
    {
        defineAttributeWithPadding(name, glTypeOf<DataT>::type, glTypeOf<DataT>::size, padding,
                                   ArrayBufferAttribute::naturalModeOf(glTypeOf<DataT>::type), 0);
    }
    template <typename DataT>
    void defineAttributeWithPadding(std::string const& name, GLuint padding, AttributeMode mode, GLuint divisor = 0)
    {
        defineAttributeWithPadding(name, glTypeOf<DataT>::type, glTypeOf<DataT>::size, padding, mode, divisor);
    }
    /// Attribute definition with offset via template
    template <typename DataT>
    void defineAttributeWithOffset(std::string const& name, GLuint offset)
    {
        defineAttributeWithOffset(name, glTypeOf<DataT>::type, glTypeOf<DataT>::size, offset,
                                  ArrayBufferAttribute::naturalModeOf(glTypeOf<DataT>::type), 0);
    }
    template <typename DataT>
    void defineAttributeWithOffset(std::string const& name, GLuint offset, AttributeMode mode, GLuint divisor = 0)
    {
        defineAttributeWithOffset(name, glTypeOf<DataT>::type, glTypeOf<DataT>::size, offset, mode, divisor);
    }

    /// Attribute definition via pointer-to-member
    /// Usage:
    ///   ab->defineAttribute(&Vertex::pos, "aPosition");
    ///
    /// Tip for 32bit colors:
    ///   ab->defineAttribute(&Point::color, "aColor", GL_UNSIGNED_BYTE, 4, AttributeMode::NormalizedInteger);
    template <class DataT, class VertexT>
    void defineAttribute(DataT VertexT::*member, std::string const& name)
    {
        if (getStride() > 0 && getStride() != sizeof(VertexT))
            ::glow::warning() << "Stride mismatch! Expected " << sizeof(VertexT) << ", got " << getStride() << " "
                              << to_string(this);
        setStride(sizeof(VertexT));

        auto offset = reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<VertexT const volatile*>(0)->*member));
        defineAttributeWithOffset(name, glTypeOf<DataT>::type, glTypeOf<DataT>::size, offset,
                                  ArrayBufferAttribute::naturalModeOf(glTypeOf<DataT>::type), 0);
    }
    template <class DataT, class VertexT>
    void defineAttribute(DataT VertexT::*member, std::string const& name, AttributeMode mode, GLuint divisor = 0)
    {
        if (getStride() > 0 && getStride() != sizeof(VertexT))
            ::glow::warning() << "Stride mismatch! Expected " << sizeof(VertexT) << ", got " << getStride() << " "
                              << to_string(this);
        setStride(sizeof(VertexT));

        auto offset = reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<VertexT const volatile*>(0)->*member));
        defineAttributeWithOffset(name, glTypeOf<DataT>::type, glTypeOf<DataT>::size, offset, mode, divisor);
    }
    template <class DataT, class VertexT>
    void defineAttribute(DataT VertexT::*member, std::string const& name, GLenum type, int elements, AttributeMode mode, GLuint divisor = 0)
    {
        if (getStride() > 0 && getStride() != sizeof(VertexT))
            ::glow::warning() << "Stride mismatch! Expected " << sizeof(VertexT) << ", got " << getStride() << " "
                              << to_string(this);
        setStride(sizeof(VertexT));

        auto offset = reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<VertexT const volatile*>(0)->*member));
        defineAttributeWithOffset(name, type, elements, offset, mode, divisor);
    }

    /// Sets the divisor of _all_ attributes
    /// CAUTION: Only modifies already defined attributes
    void setDivisor(int div);

public:
    ArrayBuffer();

    /// Binds this array buffer.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundArrayBuffer bind() { return {this}; }

public: // static construction
    /// Creates an empty array buffer
    /// Same as std::make_shared<ArrayBuffer>();
    static SharedArrayBuffer create();
    /// Creates an empty array buffer with the given attribute definitions
    /// Example:
    ///   ArrayBuffer::create({{&MyVertex::aPosition, "aPosition"},
    ///                        {&MyVertex::texCoord, "aTexCoord"}});
    static SharedArrayBuffer create(std::vector<ArrayBufferAttribute> const& attrs);
    /// Same as create(attrs) but also sets vertices immediately
    template <typename DataT>
    static SharedArrayBuffer create(std::vector<ArrayBufferAttribute> const& attrs, std::vector<DataT> const& vertices)
    {
        auto ab = create(attrs);
        ab->bind().setData(vertices);
        return ab;
    }
};
}
