#pragma once

#include <string>

#include <glow/common/gltypeinfo.hh>
#include <glow/common/ogl_typeinfo.hh>

#include <glow/gl.hh>

namespace glow
{
/// Defines the shader-access of an attribute
enum class AttributeMode
{
    Float,
    /// float, but integer data is converted to [-1, 1] (signed) or [0,1] (unsigned)
    NormalizedInteger,
    Integer,
    Double
};

/// A single named attribute of the array buffer
struct ArrayBufferAttribute
{
    std::string name;   ///< name of the attribute (e.g. "aPosition" for "in vec4 aPosition;")
    GLenum type;        ///< data type in vertex buffer, e.g. GL_FLOAT, GL_BYTE, GL_INT, ...
    GLint size;         ///< number of components: 1, 2, 3, 4, or GL_BGRA
    GLuint offset;      ///< offset into array buffer
    GLuint divisor;     ///< vertex divisor for instancing (OGL 3.3+), 0 means off/none
    AttributeMode mode; ///< type in shader (float, normalized-integer, integer, double)
    GLuint fixedStride; ///< if non-zero this dictates a stride

    ArrayBufferAttribute() = default;
    ArrayBufferAttribute(std::string const& name, GLenum type, GLint size, GLuint offset, GLuint divisor = 0, AttributeMode mode = AttributeMode::Float)
      : name(name), type(type), size(size), offset(offset), divisor(divisor), mode(mode), fixedStride(0)
    {
    }

    /// Attribute definition via pointer-to-member
    /// Usage:
    ///   ab->defineAttribute(&Vertex::pos, "aPosition");
    template <class DataT, class VertexT>
    ArrayBufferAttribute(DataT VertexT::*member, std::string const& name)
      : name(name),
        type(glTypeOf<DataT>::type),
        size(glTypeOf<DataT>::size),
        offset(reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<VertexT const volatile*>(0)->*member))),
        divisor(0),
        mode(naturalModeOf(glTypeOf<DataT>::type)),
        fixedStride(sizeof(VertexT))
    {
    }
    template <class DataT, class VertexT>
    ArrayBufferAttribute(DataT VertexT::*member, std::string const& name, AttributeMode mode, GLuint divisor = 0)
      : name(name),
        type(glTypeOf<DataT>::type),
        size(glTypeOf<DataT>::size),
        offset(reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<VertexT const volatile*>(0)->*member))),
        divisor(divisor),
        mode(mode),
        fixedStride(sizeof(VertexT))
    {
    }

    /// Returns the natural mode of a given opengl type
    /// Integer for int-types
    /// Float for float
    /// Double for double
    /// NormalizedInteger not supported
    static AttributeMode naturalModeOf(GLenum type)
    {
        switch (type)
        {
        case GL_FLOAT:
            return AttributeMode::Float;
        case GL_DOUBLE:
            return AttributeMode::Double;
        default:
            return AttributeMode::Integer;
        }
    }
};
}
