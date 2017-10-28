#pragma once


#ifdef _WIN32
#include "Win32/Window.hpp"
#else
static_assert(false, "Window system is not implemented on this platform.");
#endif