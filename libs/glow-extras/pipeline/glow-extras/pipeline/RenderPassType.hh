#pragma once

namespace glow
{
namespace pipeline
{
enum class RenderPassType
{
    /// Only output depth values for zPre, normal depth test
    ZPre,
    /// Output fColor, normal depth test
    Opaque,
    /// Use special transparency.glsl header for weighted OIT
    Transparent,
    /// TODO
    Shadow,
    /// Postprocess (no dept test, no culling) after opaque, before transparency resolve
    PostprocessOpaque,
    /// Postprocess (no dept test, no culling) after transparency resolve, in linear space
    PostprocessAll,
};
}
}
