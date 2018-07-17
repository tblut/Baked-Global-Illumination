#pragma once

// including only vec4.hpp or only glm.hpp and so on does not work for alignment!
// for whatever reason :(
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstdint>

#include "common/alignment.hh"

/** This header implements the std140 layout functionality
 * See https://www.opengl.org/registry/specs/ARB/uniform_buffer_object.txt
 *
 * TODO: Allow file reuse in shader
 *
 * Usage: (all supported types)
 *
 * struct MyUniformBufferStruct
 * {
 *  // basic types
 *  std140bool     i;
 *  std140int      j;
 *  std140unsigned k;
 *  std140float    l;
 *
 *  // vectors
 *  std140vec2 v2;
 *  std140vec3 v3;
 *  std140vec4 v4;
 *
 *  std140ivec2 iv2;
 *  std140ivec3 iv3;
 *  std140ivec4 iv4;
 *
 *  std140bvec2 bv2;
 *  std140bvec3 bv3;
 *  std140bvec4 bv4;
 *
 *  std140dvec2 dv2;
 *  std140dvec3 dv3;
 *  std140dvec4 dv4;
 *
 *  std140uvec2 uv2;
 *  std140uvec3 uv3;
 *  std140uvec4 uv4;
 *
 *  // matrices
 *  std140mat2 m2;
 *  std140mat3 m3;
 *  std140mat4 m4;
 *
 *  std140mat2x2 m2x2;
 *  std140mat2x3 m2x3;
 *  std140mat2x4 m2x4;
 *  std140mat3x2 m3x2;
 *  std140mat3x3 m3x3;
 *  std140mat3x4 m3x4;
 *  std140mat4x2 m4x2;
 *  std140mat4x3 m4x3;
 *  std140mat4x4 m4x4;
 *
 *  std140dmat2 dm2;
 *  std140dmat3 dm3;
 *  std140dmat4 dm4;
 *
 *  std140dmat2x2 dm2x2;
 *  std140dmat2x3 dm2x3;
 *  std140dmat2x4 dm2x4;
 *  std140dmat3x2 dm3x2;
 *  std140dmat3x3 dm3x3;
 *  std140dmat3x4 dm3x4;
 *  std140dmat4x2 dm4x2;
 *  std140dmat4x3 dm4x3;
 *  std140dmat4x4 dm4x4;
 *
 *  // arrays
 *  // TODO
 *
 *  // structs
 *  // TODO
 * }
 *
 * CAUTION: even if it works for some types, you should ALWAYS use std140xyz for ALL members
 *
 * Writing and reading from these variables may be a bit slower due to awkward memory layout
 *
 * You cannot calculate directly on these wrapper types, BUT an implicit conversion exists
 */

namespace glow
{
#define GLOW_STD140_AUTO_TYPE(NAME, TYPE, ALIGN_IN_FLOATS) GLOW_STD140_EXPLICIT_TYPE(NAME, TYPE, TYPE, ALIGN_IN_FLOATS)
#define GLOW_STD140_EXPLICIT_TYPE(NAME, ACCESS_T, STORE_T, ALIGN_IN_FLOATS)   \
    namespace internal                                                        \
    {                                                                         \
    struct std140##NAME##_impl                                                \
    {                                                                         \
        std140##NAME##_impl() = default;                                      \
        std140##NAME##_impl(std140##NAME##_impl const&) = default;            \
        std140##NAME##_impl(std140##NAME##_impl&&) = default;                 \
                                                                              \
        std140##NAME##_impl& operator=(std140##NAME##_impl const&) = default; \
        std140##NAME##_impl& operator=(std140##NAME##_impl&&) = default;      \
                                                                              \
        std140##NAME##_impl(ACCESS_T const& t) : val(t) {}                    \
        operator ACCESS_T() const { return (ACCESS_T)val; }                   \
        std140##NAME##_impl& operator=(ACCESS_T const& t)                     \
        {                                                                     \
            val = STORE_T(t);                                                 \
            return *this;                                                     \
        }                                                                     \
        std140##NAME##_impl& operator=(ACCESS_T&& t)                          \
        {                                                                     \
            val = STORE_T(t);                                                 \
            return *this;                                                     \
        }                                                                     \
                                                                              \
    private:                                                                  \
        STORE_T val;                                                          \
    };                                                                        \
    }                                                                         \
    typedef GLOW_ALIGN_PRE(ALIGN_IN_FLOATS * 4)                               \
        internal::std140##NAME##_impl std140##NAME GLOW_ALIGN_POST(ALIGN_IN_FLOATS * 4) // force ;

// scalars
GLOW_STD140_AUTO_TYPE(int, int32_t, 1);
GLOW_STD140_AUTO_TYPE(uint, uint32_t, 1);
GLOW_STD140_AUTO_TYPE(bool, bool, 1);
GLOW_STD140_AUTO_TYPE(float, float, 1);
GLOW_STD140_AUTO_TYPE(double, double, 2);

// vectors
GLOW_STD140_AUTO_TYPE(vec2, glm::vec2, 2);
GLOW_STD140_AUTO_TYPE(vec3, glm::vec3, 4);
GLOW_STD140_AUTO_TYPE(vec4, glm::vec4, 4);

GLOW_STD140_EXPLICIT_TYPE(bvec2, glm::bvec2, glm::ivec2, 2);
GLOW_STD140_EXPLICIT_TYPE(bvec3, glm::bvec3, glm::ivec3, 4);
GLOW_STD140_EXPLICIT_TYPE(bvec4, glm::bvec4, glm::ivec4, 4);

GLOW_STD140_AUTO_TYPE(ivec2, glm::ivec2, 2);
GLOW_STD140_AUTO_TYPE(ivec3, glm::ivec3, 4);
GLOW_STD140_AUTO_TYPE(ivec4, glm::ivec4, 4);

GLOW_STD140_AUTO_TYPE(uvec2, glm::uvec2, 2);
GLOW_STD140_AUTO_TYPE(uvec3, glm::uvec3, 4);
GLOW_STD140_AUTO_TYPE(uvec4, glm::uvec4, 4);

GLOW_STD140_AUTO_TYPE(dvec2, glm::dvec2, 2 * 2);
GLOW_STD140_AUTO_TYPE(dvec3, glm::dvec3, 4 * 2);
GLOW_STD140_AUTO_TYPE(dvec4, glm::dvec4, 4 * 2);

// matrices
GLOW_STD140_EXPLICIT_TYPE(mat2, glm::mat2, glm::mat2x4, 4);
GLOW_STD140_EXPLICIT_TYPE(mat3, glm::mat3, glm::mat3x4, 4);
GLOW_STD140_EXPLICIT_TYPE(mat4, glm::mat4, glm::mat4x4, 4);

GLOW_STD140_EXPLICIT_TYPE(mat2x2, glm::mat2x2, glm::mat2x4, 4);
GLOW_STD140_EXPLICIT_TYPE(mat2x3, glm::mat2x3, glm::mat2x4, 4);
GLOW_STD140_EXPLICIT_TYPE(mat2x4, glm::mat2x4, glm::mat2x4, 4);

GLOW_STD140_EXPLICIT_TYPE(mat3x2, glm::mat3x2, glm::mat3x4, 4);
GLOW_STD140_EXPLICIT_TYPE(mat3x3, glm::mat3x3, glm::mat3x4, 4);
GLOW_STD140_EXPLICIT_TYPE(mat3x4, glm::mat3x4, glm::mat3x4, 4);

GLOW_STD140_EXPLICIT_TYPE(mat4x2, glm::mat4x2, glm::mat4x4, 4);
GLOW_STD140_EXPLICIT_TYPE(mat4x3, glm::mat4x3, glm::mat4x4, 4);
GLOW_STD140_EXPLICIT_TYPE(mat4x4, glm::mat4x4, glm::mat4x4, 4);

GLOW_STD140_EXPLICIT_TYPE(dmat2, glm::dmat2, glm::dmat2x2, 2 * 2);
GLOW_STD140_EXPLICIT_TYPE(dmat3, glm::dmat3, glm::dmat3x4, 4 * 2);
GLOW_STD140_EXPLICIT_TYPE(dmat4, glm::dmat4, glm::dmat4x4, 4 * 2);

GLOW_STD140_EXPLICIT_TYPE(dmat2x2, glm::dmat2x2, glm::dmat2x2, 2 * 2);
GLOW_STD140_EXPLICIT_TYPE(dmat2x3, glm::dmat2x3, glm::dmat2x4, 4 * 2);
GLOW_STD140_EXPLICIT_TYPE(dmat2x4, glm::dmat2x4, glm::dmat2x4, 4 * 2);

GLOW_STD140_EXPLICIT_TYPE(dmat3x2, glm::dmat3x2, glm::dmat3x2, 2 * 2);
GLOW_STD140_EXPLICIT_TYPE(dmat3x3, glm::dmat3x3, glm::dmat3x4, 4 * 2);
GLOW_STD140_EXPLICIT_TYPE(dmat3x4, glm::dmat3x4, glm::dmat3x4, 4 * 2);

GLOW_STD140_EXPLICIT_TYPE(dmat4x2, glm::dmat4x2, glm::dmat4x2, 2 * 2);
GLOW_STD140_EXPLICIT_TYPE(dmat4x3, glm::dmat4x3, glm::dmat4x4, 4 * 2);
GLOW_STD140_EXPLICIT_TYPE(dmat4x4, glm::dmat4x4, glm::dmat4x4, 4 * 2);
}
