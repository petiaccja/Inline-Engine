#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d12.h>
#include "DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {


void ThrowIfFailed(HRESULT code);


} // namespace gxapi_dx12
} // namespace inl
