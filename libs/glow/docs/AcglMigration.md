# ACGL Migration Guide

## Namespace

All basic functionality is now in the `glow` namespace.
Extra functionality (camera, timing, ...) is in the `glow-extras-xyz` repository and usually in the `glow::xyz` namespace.

## Classes and class names

Notable renamings:

* `VertexArrayObject` -> `VertexArray`
* `FrameBufferObject` -> `Framebuffer`
* `ShaderProgram` -> `Program`

Include via

`#include <glow/objects/ObjectName.hh>`

**IMPORTANT**: Try to keep the header as include-free as possible.
You can use `#include <glow/fwd.hh>` for forward declarations of all important objects and their *SharedXyz* variants.
