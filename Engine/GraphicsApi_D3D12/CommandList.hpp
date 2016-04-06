#pragma once

#include "../GraphicsApi_LL/ICommandList.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {


class CommandList : virtual public gxapi::ICommandList {
public:
	virtual ~CommandList() = default;
	virtual ID3D12CommandList* GetNativeGenericList() const = 0;

	virtual gxapi::eCommandListType GetType() const override = 0;
};


} // namespace gxapi_dx12
} // namespace inl
