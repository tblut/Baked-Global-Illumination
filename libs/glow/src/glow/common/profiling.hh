#pragma once

// profiling specific macros

// =======================================
// AION profiling
#ifdef GLOW_AION_PROFILING

#include <aion/Action.hh>
#define GLOW_ACTION(...) ACTION(__VA_ARGS__)

#else // deactivate

#define GLOW_ACTION(...) (void)0 // force ;

#endif
