#pragma once

#define AION_MACRO_JOIN_IMPL(arg1, arg2) arg1##arg2
#define AION_MACRO_JOIN(arg1, arg2) AION_MACRO_JOIN_IMPL(arg1, arg2)
