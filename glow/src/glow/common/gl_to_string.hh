#pragma once

/// conversions for opengl enums/values to strings

#include <glow/gl.hh>
#include <string>

namespace glow
{
inline std::string glShaderTypeToString(GLenum type)
{
    switch (type)
    {
    case GL_COMPUTE_SHADER:
        return "GL_COMPUTE_SHADER";
    case GL_VERTEX_SHADER:
        return "GL_VERTEX_SHADER";
    case GL_TESS_CONTROL_SHADER:
        return "GL_TESS_CONTROL_SHADER";
    case GL_TESS_EVALUATION_SHADER:
        return "GL_TESS_EVALUATION_SHADER";
    case GL_GEOMETRY_SHADER:
        return "GL_GEOMETRY_SHADER";
    case GL_FRAGMENT_SHADER:
        return "GL_FRAGMENT_SHADER";
    default:
        return "Invalid";
    }
}
}
