#pragma once

#define GLOW_MACRO_JOIN_IMPL(arg1, arg2) arg1##arg2
#define GLOW_MACRO_JOIN(arg1, arg2) GLOW_MACRO_JOIN_IMPL(arg1, arg2)
