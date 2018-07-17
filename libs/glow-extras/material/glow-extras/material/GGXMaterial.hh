#pragma once

#include <glow/std140.hh>

namespace glow
{
namespace material
{
struct GGXMaterial
{
    std140vec3 color;
    std140float roughness;
    std140float metallic;
};
}
}
