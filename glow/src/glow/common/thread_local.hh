#pragma once

#ifdef _MSC_VER
#define GLOW_THREADLOCAL __declspec(thread)
#else
#define GLOW_THREADLOCAL __thread // GCC 4.7 has no thread_local yet
#endif
