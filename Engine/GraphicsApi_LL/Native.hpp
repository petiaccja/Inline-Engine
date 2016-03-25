#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)

#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef _WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef GENERIC_READ
#undef DOMAIN

namespace inl {
namespace gxapi {

using NativeWindowHandle = HWND;

}
}


#else

static_assert(false, "Graphics API Low-Level not supported on this platform.");

#endif