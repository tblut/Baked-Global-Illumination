#pragma once

/// This header includes forward declarations for all important glow classes
/// (Mainly objects and data)

#include <glow/common/shared.hh>

namespace glow
{
// data
GLOW_SHARED(class, TextureData);
GLOW_SHARED(class, SurfaceData);

// objects
GLOW_SHARED(class, Buffer);
GLOW_SHARED(class, ArrayBuffer);
GLOW_SHARED(class, ElementArrayBuffer);
GLOW_SHARED(class, UniformBuffer);
GLOW_SHARED(class, ShaderStorageBuffer);
GLOW_SHARED(class, AtomicCounterBuffer);

GLOW_SHARED(class, VertexArray);

GLOW_SHARED(class, Shader);
GLOW_SHARED(class, Program);

GLOW_SHARED(class, Framebuffer);

GLOW_SHARED(class, TransformFeedback);

GLOW_SHARED(class, Query);
GLOW_SHARED(class, TimerQuery);
GLOW_SHARED(class, OcclusionQuery);
GLOW_SHARED(class, PrimitiveQuery);

GLOW_SHARED(class, Texture);
GLOW_SHARED(class, Texture1D);
GLOW_SHARED(class, Texture1DArray);
GLOW_SHARED(class, Texture2D);
GLOW_SHARED(class, Texture2DArray);
GLOW_SHARED(class, Texture2DMultisample);
GLOW_SHARED(class, Texture2DMultisampleArray);
GLOW_SHARED(class, Texture3D);
GLOW_SHARED(class, TextureBuffer);
GLOW_SHARED(class, TextureCubeMap);
GLOW_SHARED(class, TextureCubeMapArray);
GLOW_SHARED(class, TextureRectangle);
}
