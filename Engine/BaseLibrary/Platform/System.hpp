#pragma once


#ifdef _WIN32
#include "Win32/System.hpp"
#else
static_assert(false, "System utlities are not implemented on this platform.");
#endif