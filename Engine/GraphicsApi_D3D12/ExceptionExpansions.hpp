#pragma once

#include <string>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../GraphicsApi_LL/DisableWin32Macros.h"

#include <d3d12.h>

namespace inl::gxapi_dx12 {


void ThrowIfFailed(HRESULT code, const std::string& additionalInfo = std::string{});


} // namespace inl::gxapi_dx12
