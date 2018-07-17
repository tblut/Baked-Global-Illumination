#pragma once

#include <cassert>

#include "glow/gl.hh"

namespace glow
{
/// Returns the size in bytes of a given OpenGL type
/// e.g. 4 for GL_FLOAT
inline GLuint sizeOfTypeInBytes(GLenum type)
{
    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
        return sizeof(uint8_t);

    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
        return sizeof(uint16_t);

    case GL_INT:
    case GL_UNSIGNED_INT:
        return sizeof(uint32_t);

    case GL_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_10F_11F_11F_REV:
        return sizeof(uint32_t);

    case GL_HALF_FLOAT:
        return sizeof(int16_t);

    case GL_FLOAT:
    case GL_FIXED:
        return sizeof(float);

    case GL_DOUBLE:
        return sizeof(double);

    default:
        assert(0 && "Invalid enum or missing implementation.");
        return 0;
    }
}

/// Returns the number of channels for a given OpenGL format
/// e.g. 1 for GL_RED and 3 for GL_RGB
inline GLuint channelsOfFormat(GLenum format)
{
    switch (format)
    {
    case GL_RED:
    case GL_BLUE:
    case GL_GREEN:
    case GL_RED_INTEGER:
    case GL_BLUE_INTEGER:
    case GL_GREEN_INTEGER:
    case GL_DEPTH_COMPONENT:
    case GL_STENCIL_INDEX:
        return 1;

    case GL_RG:
    case GL_RG_INTEGER:
    case GL_DEPTH_STENCIL: // ?
        return 2;

    case GL_RGB:
    case GL_RGB_INTEGER:
    case GL_BGR:
    case GL_BGR_INTEGER:
        return 3;

    case GL_RGBA:
    case GL_RGBA_INTEGER:
    case GL_BGRA:
    case GL_BGRA_INTEGER:
        return 4;

    default:
        assert(0 && "Invalid enum or missing implementation.");
        return 0;
    }
}

inline bool isIntegerInternalFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
    case GL_R8I:
    case GL_R16I:
    case GL_R32I:
    case GL_R8UI:
    case GL_R16UI:
    case GL_R32UI:
    case GL_RG8I:
    case GL_RG16I:
    case GL_RG32I:
    case GL_RG8UI:
    case GL_RG16UI:
    case GL_RG32UI:
    case GL_RGB8I:
    case GL_RGB16I:
    case GL_RGB32I:
    case GL_RGB8UI:
    case GL_RGB16UI:
    case GL_RGB32UI:
    case GL_RGBA8I:
    case GL_RGBA16I:
    case GL_RGBA32I:
    case GL_RGBA8UI:
    case GL_RGBA16UI:
    case GL_RGBA32UI:
    case GL_RGB10_A2UI:
        return true;

    default:
        return false;
    }
}

inline bool isSignedIntegerInternalFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
    case GL_R8I:
    case GL_R16I:
    case GL_R32I:
    case GL_RG8I:
    case GL_RG16I:
    case GL_RG32I:
    case GL_RGB8I:
    case GL_RGB16I:
    case GL_RGB32I:
    case GL_RGBA8I:
    case GL_RGBA16I:
    case GL_RGBA32I:
        return true;

    default:
        return false;
    }
}

inline bool isUnsignedIntegerInternalFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
    case GL_R8UI:
    case GL_R16UI:
    case GL_R32UI:
    case GL_RG8UI:
    case GL_RG16UI:
    case GL_RG32UI:
    case GL_RGB8UI:
    case GL_RGB16UI:
    case GL_RGB32UI:
    case GL_RGBA8UI:
    case GL_RGBA16UI:
    case GL_RGBA32UI:
    case GL_RGB10_A2UI:
        return true;

    default:
        return false;
    }
}

inline bool isDepthInternalFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
    case GL_DEPTH_STENCIL:
        return true;

    default:
        return false;
    }
}

inline bool isStencilInternalFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
    case GL_DEPTH_STENCIL:
    case GL_STENCIL_INDEX:
        return true;

    default:
        return false;
    }
}

/// Returns the base internal format of a given (potentially sized) internal format
/// e.g. returns GL_RGB for GL_R3_G3_B2
inline GLenum baseInternalFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
    case GL_R8I:
    case GL_R16I:
    case GL_R32I:
    case GL_R8UI:
    case GL_R16UI:
    case GL_R32UI:
    case GL_COMPRESSED_RED:
        return GL_RED;

    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    case GL_RG8I:
    case GL_RG16I:
    case GL_RG32I:
    case GL_RG8UI:
    case GL_RG16UI:
    case GL_RG32UI:
    case GL_COMPRESSED_RG:
    case GL_COMPRESSED_RG_RGTC2:
    case GL_COMPRESSED_SIGNED_RG_RGTC2:
        return GL_RG;

    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_SRGB8:
    case GL_RGB8_SNORM:
    case GL_RGB12:
    case GL_RGB16:
    case GL_RGB16_SNORM:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_RGB8I:
    case GL_RGB16I:
    case GL_RGB32I:
    case GL_RGB8UI:
    case GL_RGB16UI:
    case GL_RGB32UI:
    case GL_R3_G3_B2:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_COMPRESSED_RGB:
    case GL_COMPRESSED_SRGB:
#if GLOW_OPENGL_VERSION >= 42
    case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
    case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
#endif
        return GL_RGB;

    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_SRGB8_ALPHA8:
    case GL_RGB10_A2:
    case GL_RGB10_A2UI:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGBA16_SNORM:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_RGBA8I:
    case GL_RGBA16I:
    case GL_RGBA32I:
    case GL_RGBA8UI:
    case GL_RGBA16UI:
    case GL_RGBA32UI:
    case GL_COMPRESSED_RGBA:
    case GL_COMPRESSED_SRGB_ALPHA:
#if GLOW_OPENGL_VERSION >= 42
    case GL_COMPRESSED_RGBA_BPTC_UNORM:
    case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
#endif
        return GL_RGBA;

    default:
        return internalFormat;
    }
}

/// Returns the number of channels for a given OpenGL INTERNAL format
/// e.g. 1 for GL_RED and 3 for GL_RGB8UI
inline GLuint channelsOfInternalFormat(GLenum internalFormat)
{
    auto baseFormat = baseInternalFormat(internalFormat);
    switch (baseFormat)
    {
    case GL_RED:
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
        return 1;

    case GL_RG:
    case GL_DEPTH_STENCIL: // ?
        return 2;

    case GL_RGB:
    case GL_BGR:
    case GL_SRGB:
        return 3;

    case GL_RGBA:
    case GL_BGRA:
    case GL_SRGB_ALPHA:
        return 4;

    default:
        assert(0 && "Invalid enum or missing implementation.");
        return 0;
    }
}

/// Returns the number of base type components of the given uniform type
/// e.g. 12 for GL_FLOAT_MAT3x4
inline GLint componentsOfUniformType(GLenum type)
{
    switch (type)
    {
    // single comps
    case GL_FLOAT:
    case GL_DOUBLE:
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_ATOMIC_COUNTER:
    case GL_BOOL:
    case GL_INT:
        return 1;

    // sampler
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_1D_ARRAY:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_1D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_MULTISAMPLE:
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_SAMPLER_BUFFER:
    case GL_SAMPLER_2D_RECT:
    case GL_SAMPLER_2D_RECT_SHADOW:
    case GL_INT_SAMPLER_1D:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_3D:
    case GL_INT_SAMPLER_CUBE:
    case GL_INT_SAMPLER_1D_ARRAY:
    case GL_INT_SAMPLER_2D_ARRAY:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_INT_SAMPLER_BUFFER:
    case GL_INT_SAMPLER_2D_RECT:
    case GL_UNSIGNED_INT_SAMPLER_1D:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_3D:
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        return 1;

    // images
    case GL_IMAGE_1D:
    case GL_IMAGE_2D:
    case GL_IMAGE_3D:
    case GL_IMAGE_CUBE:
    case GL_IMAGE_1D_ARRAY:
    case GL_IMAGE_2D_ARRAY:
    case GL_IMAGE_2D_MULTISAMPLE:
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_IMAGE_BUFFER:
    case GL_IMAGE_2D_RECT:
    case GL_INT_IMAGE_1D:
    case GL_INT_IMAGE_2D:
    case GL_INT_IMAGE_3D:
    case GL_INT_IMAGE_CUBE:
    case GL_INT_IMAGE_1D_ARRAY:
    case GL_INT_IMAGE_2D_ARRAY:
    case GL_INT_IMAGE_2D_MULTISAMPLE:
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_INT_IMAGE_BUFFER:
    case GL_INT_IMAGE_2D_RECT:
    case GL_UNSIGNED_INT_IMAGE_1D:
    case GL_UNSIGNED_INT_IMAGE_2D:
    case GL_UNSIGNED_INT_IMAGE_3D:
    case GL_UNSIGNED_INT_IMAGE_CUBE:
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_BUFFER:
    case GL_UNSIGNED_INT_IMAGE_2D_RECT:
        return 1;

    // vecs
    case GL_FLOAT_VEC2:
    case GL_DOUBLE_VEC2:
    case GL_UNSIGNED_INT_VEC2:
    case GL_INT_VEC2:
    case GL_BOOL_VEC2:
        return 2;

    case GL_FLOAT_VEC3:
    case GL_DOUBLE_VEC3:
    case GL_UNSIGNED_INT_VEC3:
    case GL_INT_VEC3:
    case GL_BOOL_VEC3:
        return 3;

    case GL_FLOAT_VEC4:
    case GL_DOUBLE_VEC4:
    case GL_UNSIGNED_INT_VEC4:
    case GL_INT_VEC4:
    case GL_BOOL_VEC4:
        return 4;

    // mats
    case GL_FLOAT_MAT2:
    case GL_DOUBLE_MAT2:
        return 2 * 2;

    case GL_FLOAT_MAT3:
    case GL_DOUBLE_MAT3:
        return 3 * 3;

    case GL_FLOAT_MAT4:
    case GL_DOUBLE_MAT4:
        return 4 * 4;

    case GL_FLOAT_MAT2x3:
    case GL_DOUBLE_MAT2x3:
        return 2 * 3;

    case GL_FLOAT_MAT2x4:
    case GL_DOUBLE_MAT2x4:
        return 2 * 4;

    case GL_FLOAT_MAT3x2:
    case GL_DOUBLE_MAT3x2:
        return 3 * 2;

    case GL_FLOAT_MAT3x4:
    case GL_DOUBLE_MAT3x4:
        return 3 * 4;

    case GL_FLOAT_MAT4x2:
    case GL_DOUBLE_MAT4x2:
        return 4 * 2;

    case GL_FLOAT_MAT4x3:
    case GL_DOUBLE_MAT4x3:
        return 4 * 3;

    default:
        assert(0 && "Invalid enum or missing implementation.");
        return 0;
    }
}

inline size_t byteSizeOfUniformType(GLenum uniformType)
{
    switch (uniformType)
    {
    // floats
    case GL_FLOAT:
    case GL_FLOAT_VEC2:
    case GL_FLOAT_VEC3:
    case GL_FLOAT_VEC4:
    case GL_FLOAT_MAT2:
    case GL_FLOAT_MAT3:
    case GL_FLOAT_MAT4:
    case GL_FLOAT_MAT2x3:
    case GL_FLOAT_MAT2x4:
    case GL_FLOAT_MAT3x2:
    case GL_FLOAT_MAT3x4:
    case GL_FLOAT_MAT4x2:
    case GL_FLOAT_MAT4x3:
        return sizeof(float) * componentsOfUniformType(uniformType);

    // doubles
    case GL_DOUBLE:
    case GL_DOUBLE_VEC2:
    case GL_DOUBLE_VEC3:
    case GL_DOUBLE_VEC4:
    case GL_DOUBLE_MAT2:
    case GL_DOUBLE_MAT3:
    case GL_DOUBLE_MAT4:
    case GL_DOUBLE_MAT2x3:
    case GL_DOUBLE_MAT2x4:
    case GL_DOUBLE_MAT3x2:
    case GL_DOUBLE_MAT3x4:
    case GL_DOUBLE_MAT4x2:
    case GL_DOUBLE_MAT4x3:
        return sizeof(double) * componentsOfUniformType(uniformType);

    // uint
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_VEC2:
    case GL_UNSIGNED_INT_VEC3:
    case GL_UNSIGNED_INT_VEC4:
    case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        return sizeof(uint32_t) * componentsOfUniformType(uniformType);

    // int
    case GL_INT:
    case GL_INT_VEC2:
    case GL_INT_VEC3:
    case GL_INT_VEC4:
    // bool
    case GL_BOOL:
    case GL_BOOL_VEC2:
    case GL_BOOL_VEC3:
    case GL_BOOL_VEC4:
    // sampler
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_1D_ARRAY:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_1D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_MULTISAMPLE:
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_SAMPLER_BUFFER:
    case GL_SAMPLER_2D_RECT:
    case GL_SAMPLER_2D_RECT_SHADOW:
    case GL_INT_SAMPLER_1D:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_3D:
    case GL_INT_SAMPLER_CUBE:
    case GL_INT_SAMPLER_1D_ARRAY:
    case GL_INT_SAMPLER_2D_ARRAY:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_INT_SAMPLER_BUFFER:
    case GL_INT_SAMPLER_2D_RECT:
    case GL_UNSIGNED_INT_SAMPLER_1D:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_3D:
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
    // images
    case GL_IMAGE_1D:
    case GL_IMAGE_2D:
    case GL_IMAGE_3D:
    case GL_IMAGE_CUBE:
    case GL_IMAGE_1D_ARRAY:
    case GL_IMAGE_2D_ARRAY:
    case GL_IMAGE_2D_MULTISAMPLE:
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_IMAGE_BUFFER:
    case GL_IMAGE_2D_RECT:
    case GL_INT_IMAGE_1D:
    case GL_INT_IMAGE_2D:
    case GL_INT_IMAGE_3D:
    case GL_INT_IMAGE_CUBE:
    case GL_INT_IMAGE_1D_ARRAY:
    case GL_INT_IMAGE_2D_ARRAY:
    case GL_INT_IMAGE_2D_MULTISAMPLE:
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_INT_IMAGE_BUFFER:
    case GL_INT_IMAGE_2D_RECT:
    case GL_UNSIGNED_INT_IMAGE_1D:
    case GL_UNSIGNED_INT_IMAGE_2D:
    case GL_UNSIGNED_INT_IMAGE_3D:
    case GL_UNSIGNED_INT_IMAGE_CUBE:
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_BUFFER:
    case GL_UNSIGNED_INT_IMAGE_2D_RECT:
        return sizeof(int32_t) * componentsOfUniformType(uniformType);

    // rest is int
    default:
        assert(0 && "Invalid enum or missing implementation.");
        return 0;
    }
}

inline void getUniformValue(GLenum uniformType, GLuint objName, GLuint uniformIndex, void *dataPtr)
{
    // see https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glGetActiveUniform.xml
    switch (uniformType)
    {
    // floats
    case GL_FLOAT:
    case GL_FLOAT_VEC2:
    case GL_FLOAT_VEC3:
    case GL_FLOAT_VEC4:
    case GL_FLOAT_MAT2:
    case GL_FLOAT_MAT3:
    case GL_FLOAT_MAT4:
    case GL_FLOAT_MAT2x3:
    case GL_FLOAT_MAT2x4:
    case GL_FLOAT_MAT3x2:
    case GL_FLOAT_MAT3x4:
    case GL_FLOAT_MAT4x2:
    case GL_FLOAT_MAT4x3:
        glGetUniformfv(objName, uniformIndex, (GLfloat *)dataPtr);
        break;

    // doubles
    case GL_DOUBLE:
    case GL_DOUBLE_VEC2:
    case GL_DOUBLE_VEC3:
    case GL_DOUBLE_VEC4:
    case GL_DOUBLE_MAT2:
    case GL_DOUBLE_MAT3:
    case GL_DOUBLE_MAT4:
    case GL_DOUBLE_MAT2x3:
    case GL_DOUBLE_MAT2x4:
    case GL_DOUBLE_MAT3x2:
    case GL_DOUBLE_MAT3x4:
    case GL_DOUBLE_MAT4x2:
    case GL_DOUBLE_MAT4x3:
        glGetUniformdv(objName, uniformIndex, (GLdouble *)dataPtr);
        break;

    // uint
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_VEC2:
    case GL_UNSIGNED_INT_VEC3:
    case GL_UNSIGNED_INT_VEC4:
    case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        glGetUniformuiv(objName, uniformIndex, (GLuint *)dataPtr);
        break;

    // int
    case GL_INT:
    case GL_INT_VEC2:
    case GL_INT_VEC3:
    case GL_INT_VEC4:
    // bool
    case GL_BOOL:
    case GL_BOOL_VEC2:
    case GL_BOOL_VEC3:
    case GL_BOOL_VEC4:
    // sampler
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_1D_ARRAY:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_1D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_MULTISAMPLE:
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_SAMPLER_BUFFER:
    case GL_SAMPLER_2D_RECT:
    case GL_SAMPLER_2D_RECT_SHADOW:
    case GL_INT_SAMPLER_1D:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_3D:
    case GL_INT_SAMPLER_CUBE:
    case GL_INT_SAMPLER_1D_ARRAY:
    case GL_INT_SAMPLER_2D_ARRAY:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_INT_SAMPLER_BUFFER:
    case GL_INT_SAMPLER_2D_RECT:
    case GL_UNSIGNED_INT_SAMPLER_1D:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_3D:
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
    // images
    case GL_IMAGE_1D:
    case GL_IMAGE_2D:
    case GL_IMAGE_3D:
    case GL_IMAGE_CUBE:
    case GL_IMAGE_1D_ARRAY:
    case GL_IMAGE_2D_ARRAY:
    case GL_IMAGE_2D_MULTISAMPLE:
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_IMAGE_BUFFER:
    case GL_IMAGE_2D_RECT:
    case GL_INT_IMAGE_1D:
    case GL_INT_IMAGE_2D:
    case GL_INT_IMAGE_3D:
    case GL_INT_IMAGE_CUBE:
    case GL_INT_IMAGE_1D_ARRAY:
    case GL_INT_IMAGE_2D_ARRAY:
    case GL_INT_IMAGE_2D_MULTISAMPLE:
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_INT_IMAGE_BUFFER:
    case GL_INT_IMAGE_2D_RECT:
    case GL_UNSIGNED_INT_IMAGE_1D:
    case GL_UNSIGNED_INT_IMAGE_2D:
    case GL_UNSIGNED_INT_IMAGE_3D:
    case GL_UNSIGNED_INT_IMAGE_CUBE:
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_BUFFER:
    case GL_UNSIGNED_INT_IMAGE_2D_RECT:
        glGetUniformiv(objName, uniformIndex, (GLint *)dataPtr);
        break;

    // rest is int
    default:
        assert(0 && "Invalid enum or missing implementation.");
        break;
    }
}

inline GLenum bufferBindingOf(GLenum bufferType)
{
    switch (bufferType)
    {
    case GL_ARRAY_BUFFER:
        return GL_ARRAY_BUFFER_BINDING;
    case GL_ATOMIC_COUNTER_BUFFER:
        return GL_ATOMIC_COUNTER_BUFFER_BINDING;
    case GL_COPY_READ_BUFFER:
        return GL_COPY_READ_BUFFER_BINDING;
    case GL_COPY_WRITE_BUFFER:
        return GL_COPY_WRITE_BUFFER_BINDING;
    case GL_DRAW_INDIRECT_BUFFER:
        return GL_DRAW_INDIRECT_BUFFER_BINDING;
    case GL_DISPATCH_INDIRECT_BUFFER:
        return GL_DISPATCH_INDIRECT_BUFFER_BINDING;
    case GL_ELEMENT_ARRAY_BUFFER:
        return GL_ELEMENT_ARRAY_BUFFER_BINDING;
    case GL_PIXEL_PACK_BUFFER:
        return GL_PIXEL_PACK_BUFFER_BINDING;
    case GL_PIXEL_UNPACK_BUFFER:
        return GL_PIXEL_UNPACK_BUFFER_BINDING;
    case GL_SHADER_STORAGE_BUFFER:
        return GL_SHADER_STORAGE_BUFFER_BINDING;
    case GL_TRANSFORM_FEEDBACK_BUFFER:
        return GL_TRANSFORM_FEEDBACK_BUFFER_BINDING;
    case GL_UNIFORM_BUFFER:
        return GL_UNIFORM_BUFFER_BINDING;
    default:
        assert(0 && "Invalid enum or missing implementation.");
        return GL_INVALID_ENUM;
    }
}
}
