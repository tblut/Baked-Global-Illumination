#include "gltypeinfo.hh"

#include <glow/common/log.hh>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat2x2.hpp>
#include <glm/mat2x3.hpp>
#include <glm/mat2x4.hpp>
#include <glm/mat3x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x4.hpp>
#include <glm/mat4x2.hpp>
#include <glm/mat4x3.hpp>
#include <glm/mat4x4.hpp>

#include <cstdint>

namespace glow
{
// ==========================================================
// C++ Types
template <>
GLenum glTypeOf<int8_t>::type = GL_BYTE;
template <>
GLenum glTypeOf<int8_t>::format = GL_RED_INTEGER;
template <>
GLint glTypeOf<int8_t>::size = 1;

template <>
GLenum glTypeOf<uint8_t>::type = GL_UNSIGNED_BYTE;
template <>
GLenum glTypeOf<uint8_t>::format = GL_RED;
template <>
GLint glTypeOf<uint8_t>::size = 1;

template <>
GLenum glTypeOf<int16_t>::type = GL_SHORT;
template <>
GLenum glTypeOf<int16_t>::format = GL_RED_INTEGER;
template <>
GLint glTypeOf<int16_t>::size = 1;

template <>
GLenum glTypeOf<uint16_t>::type = GL_UNSIGNED_SHORT;
template <>
GLenum glTypeOf<uint16_t>::format = GL_RED_INTEGER;
template <>
GLint glTypeOf<uint16_t>::size = 1;

template <>
GLenum glTypeOf<int32_t>::type = GL_INT;
template <>
GLenum glTypeOf<int32_t>::format = GL_RED_INTEGER;
template <>
GLint glTypeOf<int32_t>::size = 1;
template <>
internal::glBaseType glTypeOf<int32_t>::basetype = internal::glBaseType::Int;

template <>
GLenum glTypeOf<uint32_t>::type = GL_UNSIGNED_INT;
template <>
GLenum glTypeOf<uint32_t>::format = GL_RED_INTEGER;
template <>
GLint glTypeOf<uint32_t>::size = 1;
template <>
internal::glBaseType glTypeOf<uint32_t>::basetype = internal::glBaseType::UInt;

template <>
GLenum glTypeOf<float>::type = GL_FLOAT;
template <>
GLenum glTypeOf<float>::format = GL_RED;
template <>
GLint glTypeOf<float>::size = 1;
template <>
internal::glBaseType glTypeOf<float>::basetype = internal::glBaseType::Float;

template <>
GLenum glTypeOf<double>::type = GL_DOUBLE;
template <>
GLint glTypeOf<double>::size = 1;
template <>
internal::glBaseType glTypeOf<double>::basetype = internal::glBaseType::Double;


// ==========================================================
// GLM Types
template <>
GLenum glTypeOf<glm::vec2>::type = GL_FLOAT;
template <>
GLenum glTypeOf<glm::vec2>::format = GL_RG;
template <>
GLint glTypeOf<glm::vec2>::size = 2;
template <>
internal::glBaseType glTypeOf<glm::vec2>::basetype = internal::glBaseType::Float;

template <>
GLenum glTypeOf<glm::vec3>::type = GL_FLOAT;
template <>
GLenum glTypeOf<glm::vec3>::format = GL_RGB;
template <>
GLint glTypeOf<glm::vec3>::size = 3;
template <>
internal::glBaseType glTypeOf<glm::vec3>::basetype = internal::glBaseType::Float;

template <>
GLenum glTypeOf<glm::vec4>::type = GL_FLOAT;
template <>
GLenum glTypeOf<glm::vec4>::format = GL_RGBA;
template <>
GLint glTypeOf<glm::vec4>::size = 4;
template <>
internal::glBaseType glTypeOf<glm::vec4>::basetype = internal::glBaseType::Float;


template <>
GLenum glTypeOf<glm::ivec2>::type = GL_INT;
template <>
GLenum glTypeOf<glm::ivec2>::format = GL_RG_INTEGER;
template <>
GLint glTypeOf<glm::ivec2>::size = 2;
template <>
internal::glBaseType glTypeOf<glm::ivec2>::basetype = internal::glBaseType::Int;

template <>
GLenum glTypeOf<glm::ivec3>::type = GL_INT;
template <>
GLenum glTypeOf<glm::ivec3>::format = GL_RGB_INTEGER;
template <>
GLint glTypeOf<glm::ivec3>::size = 3;
template <>
internal::glBaseType glTypeOf<glm::ivec3>::basetype = internal::glBaseType::Int;

template <>
GLenum glTypeOf<glm::ivec4>::type = GL_INT;
template <>
GLenum glTypeOf<glm::ivec4>::format = GL_RGBA_INTEGER;
template <>
GLint glTypeOf<glm::ivec4>::size = 4;
template <>
internal::glBaseType glTypeOf<glm::ivec4>::basetype = internal::glBaseType::Int;


template <>
GLenum glTypeOf<glm::uvec2>::type = GL_UNSIGNED_INT;
template <>
GLenum glTypeOf<glm::uvec2>::format = GL_RG_INTEGER;
template <>
GLint glTypeOf<glm::uvec2>::size = 2;
template <>
internal::glBaseType glTypeOf<glm::uvec2>::basetype = internal::glBaseType::UInt;

template <>
GLenum glTypeOf<glm::uvec3>::type = GL_UNSIGNED_INT;
template <>
GLenum glTypeOf<glm::uvec3>::format = GL_RGB_INTEGER;
template <>
GLint glTypeOf<glm::uvec3>::size = 3;
template <>
internal::glBaseType glTypeOf<glm::uvec3>::basetype = internal::glBaseType::UInt;

template <>
GLenum glTypeOf<glm::uvec4>::type = GL_UNSIGNED_INT;
template <>
GLenum glTypeOf<glm::uvec4>::format = GL_RGBA_INTEGER;
template <>
GLint glTypeOf<glm::uvec4>::size = 4;
template <>
internal::glBaseType glTypeOf<glm::uvec4>::basetype = internal::glBaseType::UInt;


template <>
GLenum glTypeOf<glm::dvec2>::type = GL_DOUBLE;
template <>
GLint glTypeOf<glm::dvec2>::size = 2;
template <>
internal::glBaseType glTypeOf<glm::dvec2>::basetype = internal::glBaseType::Double;

template <>
GLenum glTypeOf<glm::dvec3>::type = GL_DOUBLE;
template <>
GLint glTypeOf<glm::dvec3>::size = 3;
template <>
internal::glBaseType glTypeOf<glm::dvec3>::basetype = internal::glBaseType::Double;

template <>
GLenum glTypeOf<glm::dvec4>::type = GL_DOUBLE;
template <>
GLint glTypeOf<glm::dvec4>::size = 4;
template <>
internal::glBaseType glTypeOf<glm::dvec4>::basetype = internal::glBaseType::Double;


template <>
internal::glBaseType glTypeOf<glm::mat2x2>::basetype = internal::glBaseType::Float;
template <>
internal::glBaseType glTypeOf<glm::mat2x3>::basetype = internal::glBaseType::Float;
template <>
internal::glBaseType glTypeOf<glm::mat2x4>::basetype = internal::glBaseType::Float;
template <>
internal::glBaseType glTypeOf<glm::mat3x2>::basetype = internal::glBaseType::Float;
template <>
internal::glBaseType glTypeOf<glm::mat3x3>::basetype = internal::glBaseType::Float;
template <>
internal::glBaseType glTypeOf<glm::mat3x4>::basetype = internal::glBaseType::Float;
template <>
internal::glBaseType glTypeOf<glm::mat4x2>::basetype = internal::glBaseType::Float;
template <>
internal::glBaseType glTypeOf<glm::mat4x3>::basetype = internal::glBaseType::Float;
template <>
internal::glBaseType glTypeOf<glm::mat4x4>::basetype = internal::glBaseType::Float;

template <>
internal::glBaseType glTypeOf<glm::dmat2x2>::basetype = internal::glBaseType::Double;
template <>
internal::glBaseType glTypeOf<glm::dmat2x3>::basetype = internal::glBaseType::Double;
template <>
internal::glBaseType glTypeOf<glm::dmat2x4>::basetype = internal::glBaseType::Double;
template <>
internal::glBaseType glTypeOf<glm::dmat3x2>::basetype = internal::glBaseType::Double;
template <>
internal::glBaseType glTypeOf<glm::dmat3x3>::basetype = internal::glBaseType::Double;
template <>
internal::glBaseType glTypeOf<glm::dmat3x4>::basetype = internal::glBaseType::Double;
template <>
internal::glBaseType glTypeOf<glm::dmat4x2>::basetype = internal::glBaseType::Double;
template <>
internal::glBaseType glTypeOf<glm::dmat4x3>::basetype = internal::glBaseType::Double;
template <>
internal::glBaseType glTypeOf<glm::dmat4x4>::basetype = internal::glBaseType::Double;

// ==========================================================
// Other
std::string glUniformTypeToString(GLenum type)
{
    switch (type)
    {
    case GL_FLOAT:
        return "float";
    case GL_FLOAT_VEC2:
        return "vec2";
    case GL_FLOAT_VEC3:
        return "vec3";
    case GL_FLOAT_VEC4:
        return "vec4";
    case GL_DOUBLE:
        return "double";
    case GL_DOUBLE_VEC2:
        return "dvec2";
    case GL_DOUBLE_VEC3:
        return "dvec3";
    case GL_DOUBLE_VEC4:
        return "dvec4";
    case GL_INT:
        return "int";
    case GL_INT_VEC2:
        return "ivec2";
    case GL_INT_VEC3:
        return "ivec3";
    case GL_INT_VEC4:
        return "ivec4";
    case GL_UNSIGNED_INT:
        return "unsigned int";
    case GL_UNSIGNED_INT_VEC2:
        return "uvec2";
    case GL_UNSIGNED_INT_VEC3:
        return "uvec3";
    case GL_UNSIGNED_INT_VEC4:
        return "uvec4";
    case GL_BOOL:
        return "bool";
    case GL_BOOL_VEC2:
        return "bvec2";
    case GL_BOOL_VEC3:
        return "bvec3";
    case GL_BOOL_VEC4:
        return "bvec4";
    case GL_FLOAT_MAT2:
        return "mat2";
    case GL_FLOAT_MAT3:
        return "mat3";
    case GL_FLOAT_MAT4:
        return "mat4";
    case GL_FLOAT_MAT2x3:
        return "mat2x3";
    case GL_FLOAT_MAT2x4:
        return "mat2x4";
    case GL_FLOAT_MAT3x2:
        return "mat3x2";
    case GL_FLOAT_MAT3x4:
        return "mat3x4";
    case GL_FLOAT_MAT4x2:
        return "mat4x2";
    case GL_FLOAT_MAT4x3:
        return "mat4x3";
    case GL_DOUBLE_MAT2:
        return "dmat2";
    case GL_DOUBLE_MAT3:
        return "dmat3";
    case GL_DOUBLE_MAT4:
        return "dmat4";
    case GL_DOUBLE_MAT2x3:
        return "dmat2x3";
    case GL_DOUBLE_MAT2x4:
        return "dmat2x4";
    case GL_DOUBLE_MAT3x2:
        return "dmat3x2";
    case GL_DOUBLE_MAT3x4:
        return "dmat3x4";
    case GL_DOUBLE_MAT4x2:
        return "dmat4x2";
    case GL_DOUBLE_MAT4x3:
        return "dmat4x3";
    case GL_SAMPLER_1D:
        return "sampler1D";
    case GL_SAMPLER_2D:
        return "sampler2D";
    case GL_SAMPLER_3D:
        return "sampler3D";
    case GL_SAMPLER_CUBE:
        return "samplerCube";
    case GL_SAMPLER_1D_SHADOW:
        return "sampler1DShadow";
    case GL_SAMPLER_2D_SHADOW:
        return "sampler2DShadow";
    case GL_SAMPLER_1D_ARRAY:
        return "sampler1DArray";
    case GL_SAMPLER_2D_ARRAY:
        return "sampler2DArray";
    case GL_SAMPLER_1D_ARRAY_SHADOW:
        return "sampler1DArrayShadow";
    case GL_SAMPLER_2D_ARRAY_SHADOW:
        return "sampler2DArrayShadow";
    case GL_SAMPLER_2D_MULTISAMPLE:
        return "sampler2DMS";
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        return "sampler2DMSArray";
    case GL_SAMPLER_CUBE_SHADOW:
        return "samplerCubeShadow";
    case GL_SAMPLER_BUFFER:
        return "samplerBuffer";
    case GL_SAMPLER_2D_RECT:
        return "sampler2DRect";
    case GL_SAMPLER_2D_RECT_SHADOW:
        return "sampler2DRectShadow";
    case GL_INT_SAMPLER_1D:
        return "isampler1D";
    case GL_INT_SAMPLER_2D:
        return "isampler2D";
    case GL_INT_SAMPLER_3D:
        return "isampler3D";
    case GL_INT_SAMPLER_CUBE:
        return "isamplerCube";
    case GL_INT_SAMPLER_1D_ARRAY:
        return "isampler1DArray";
    case GL_INT_SAMPLER_2D_ARRAY:
        return "isampler2DArray";
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
        return "isampler2DMS";
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        return "isampler2DMSArray";
    case GL_INT_SAMPLER_BUFFER:
        return "isamplerBuffer";
    case GL_INT_SAMPLER_2D_RECT:
        return "isampler2DRect";
    case GL_UNSIGNED_INT_SAMPLER_1D:
        return "usampler1D";
    case GL_UNSIGNED_INT_SAMPLER_2D:
        return "usampler2D";
    case GL_UNSIGNED_INT_SAMPLER_3D:
        return "usampler3D";
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
        return "usamplerCube";
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        return "usampler2DArray";
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        return "usampler2DArray";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        return "usampler2DMS";
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        return "usampler2DMSArray";
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        return "usamplerBuffer";
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        return "usampler2DRect";
    case GL_IMAGE_1D:
        return "image1D";
    case GL_IMAGE_2D:
        return "image2D";
    case GL_IMAGE_3D:
        return "image3D";
    case GL_IMAGE_2D_RECT:
        return "image2DRect";
    case GL_IMAGE_CUBE:
        return "imageCube";
    case GL_IMAGE_BUFFER:
        return "imageBuffer";
    case GL_IMAGE_1D_ARRAY:
        return "image1DArray";
    case GL_IMAGE_2D_ARRAY:
        return "image2DArray";
    case GL_IMAGE_2D_MULTISAMPLE:
        return "image2DMS";
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
        return "image2DMSArray";
    case GL_INT_IMAGE_1D:
        return "iimage1D";
    case GL_INT_IMAGE_2D:
        return "iimage2D";
    case GL_INT_IMAGE_3D:
        return "iimage3D";
    case GL_INT_IMAGE_2D_RECT:
        return "iimage2DRect";
    case GL_INT_IMAGE_CUBE:
        return "iimageCube";
    case GL_INT_IMAGE_BUFFER:
        return "iimageBuffer";
    case GL_INT_IMAGE_1D_ARRAY:
        return "iimage1DArray";
    case GL_INT_IMAGE_2D_ARRAY:
        return "iimage2DArray";
    case GL_INT_IMAGE_2D_MULTISAMPLE:
        return "iimage2DMS";
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
        return "iimage2DMSArray";
    case GL_UNSIGNED_INT_IMAGE_1D:
        return "uimage1D";
    case GL_UNSIGNED_INT_IMAGE_2D:
        return "uimage2D";
    case GL_UNSIGNED_INT_IMAGE_3D:
        return "uimage3D";
    case GL_UNSIGNED_INT_IMAGE_2D_RECT:
        return "uimage2DRect";
    case GL_UNSIGNED_INT_IMAGE_CUBE:
        return "uimageCube";
    case GL_UNSIGNED_INT_IMAGE_BUFFER:
        return "uimageBuffer";
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
        return "uimage1DArray";
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
        return "uimage2DArray";
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
        return "uimage2DMS";
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
        return "uimage2DMSArray";
    case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        return "atomic_uint";

    default:
        error() << "Uniform type not implemented: " << type;
        return "<error>";
    }
}

std::string glUniformTypeToString(GLenum type, GLint size)
{
    if (size == 1)
        return glUniformTypeToString(type);
    else
        return glUniformTypeToString(type) + "[" + std::to_string(size) + "]";
}
}
