#pragma once

#include <glow/common/str_utils.hh>
#include <string>

#ifndef DEV_BUILD
#define DEV_BUILD
#endif

inline std::string getWorkDir() {
#ifdef DEV_BUILD
	return glow::util::pathOf(__FILE__);
#else
	return "."
#endif
}