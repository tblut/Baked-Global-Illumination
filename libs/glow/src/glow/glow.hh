#pragma once

// it's easy to mix Debug and Release on windows
// this should catch the most obvious errors
#ifdef GLOW_COMPILER_MSVC
#ifdef _DEBUG
#ifndef GLOW_DEBUG
#error Visual Studio is set to debug but CMake is not. Settings must match.
#endif
#else
#ifdef GLOW_DEBUG
#error CMake is set to debug but Visual Studio is not. Settings must match.
#endif
#endif
#endif

namespace glow
{
/// OpenGL Version information
/// Only valid after initGLOW
extern struct _glowGLVersion
{
    int major;
    int minor;
    int total; // like 43 for 4.3
} OGLVersion;

/**
 * @brief initializes the GLOW Library
 *
 * In particular:
 *   - loads OpenGL function
 *   - performs certain static initialization
 *
 * @return true on success
 */
bool initGLOW();

/// Asserts that glow is initialized
/// Fails if not initialized OR wrong thread
#ifdef GLOW_PERFORM_VALIDATION
void checkValidGLOW();
#else
#define checkValidGLOW() (void)0
#endif
}
