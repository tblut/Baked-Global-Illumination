#pragma once

#include "RenderPassType.hh"

#include <glow/fwd.hh>

namespace glow
{
namespace camera
{
class FixedCamera;
}

namespace pipeline
{
class RenderingPipeline;

/**
 * @brief Information about the current render pass
 *
 * CAREFUL: This data is temporary and may not be valid after the rendering function returns
 */
struct RenderPass
{
    /// Type of the current pass (there can be multiple passes with the same type)
    RenderPassType type;

    /// Camera parameters (including width/height) for current pass
    camera::FixedCamera* camera = nullptr;

    /// Backreference to current pipeline
    RenderingPipeline* pipeline = nullptr;

    /// Currently bound depth buffer
    SharedTextureRectangle depthTarget = nullptr;
    /// Currently bound framebuffer
    SharedFramebuffer framebuffer = nullptr;
};
}
}
