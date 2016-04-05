#pragma once

#include <string>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d12.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {


void ThrowIfFailed(HRESULT code, const std::string& additionalInfo = std::string{});


} // namespace gxapi_dx12
} // namespace inl
