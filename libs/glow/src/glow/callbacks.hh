#pragma once

namespace glow
{
/// Notifies GLOW that a shader was executed
/// This triggers some warning mechanisms:
/// - mipmaps are invalidated
/// - unused uniforms are checked (once per shader)
void notifyShaderExecuted();
}
