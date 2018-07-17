#pragma once

#include <glow/gl.hh>
#include <glow/common/shared.hh>

namespace glow
{
GLOW_SHARED(class, ArrayBuffer);

/// A location in this VAO
struct VertexArrayAttribute
{
    SharedArrayBuffer buffer;
    GLuint locationInBuffer;
    GLuint locationInVAO;
};
}
