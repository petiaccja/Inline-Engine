#pragma once

#include "../GraphicsApi_LL/ICommandList.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>
#include "DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {


class CommandList : public gxapi::ICommandList {
public:
	virtual ~CommandList() = default;
	virtual ID3D12CommandList* GetNativeGenericList() const = 0;

	gxapi::eCommandListType GetType() const override;
};


} // namespace gxapi_dx12
} // namespace inl
