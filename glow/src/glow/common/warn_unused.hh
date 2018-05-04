#pragma once

#ifdef GLOW_COMPILER_MSVC
#define GLOW_WARN_UNUSED
#else
#define GLOW_WARN_UNUSED __attribute__((warn_unused_result))
#endif
