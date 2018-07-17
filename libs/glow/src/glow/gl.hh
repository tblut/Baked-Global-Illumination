#pragma once

/// Include this header whenever you need access to OpenGL functions

#include "glad/glad.h"

namespace glow
{
/**
 * @brief restores various openGL states to their default setting
 *
 * This is not necessarily comprehensive, so better check the implementation to be sure
 *
 * Includes at least:
 *   * all glEnable/Disable states
 *   * their derived calls (glBlendFunc, ...)
 */
void restoreDefaultOpenGLState();

/**
 * @brief sets all OpenGL object bindings to 0
 *
 * CAUTION: calling it while a GLOW object is bound is undefined behavior!
 *
 * Can be used after 3rd party calls to make sure no state is accidentally changed
 */
void unbindOpenGLObjects();
}
