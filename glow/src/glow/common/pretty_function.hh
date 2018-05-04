#pragma once

/**
 * This header defines __PRETTY_FUNCTION__ that returns the current function name
 * (non-msvc has builtin support for this)
 */

#ifdef GLOW_COMPILER_MSVC

// __PRETTY_FUNCTION__ is non-msvc
#define __PRETTY_FUNCTION__ __FUNCTION__

#endif
