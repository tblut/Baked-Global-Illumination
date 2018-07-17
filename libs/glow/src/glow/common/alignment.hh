#pragma once

#ifdef GLOW_COMPILER_MSVC
#define GLOW_ALIGN_POST(BYTES)
#define GLOW_ALIGN_PRE(BYTES) __declspec(align(BYTES))
#else
#define GLOW_ALIGN_POST(BYTES) __attribute__((__aligned__(BYTES)))
#define GLOW_ALIGN_PRE(BYTES)
#endif
