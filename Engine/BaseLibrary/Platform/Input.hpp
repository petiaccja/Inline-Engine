#pragma once


#ifdef _WIN32
#include "Win32/Input.hpp"
#else
static_assert(false, "Input system is not implemented on this platform.");
#endif