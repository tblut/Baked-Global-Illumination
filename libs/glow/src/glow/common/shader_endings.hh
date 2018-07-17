#pragma once

#include "glow/gl.hh"

#include <string>
#include <map>

namespace glow
{
/// Mapping from ending (e.g. ".vsh" to shader type e.g. GL_VERTEX_SHADER
extern const std::map<std::string, GLenum> shaderEndingToType;
}
