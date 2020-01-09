#pragma once

#include "../GraphicsApi_LL/IPipelineState.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../GraphicsApi_LL/DisableWin32Macros.h"

#include <d3d12.h>
#include <wrl.h>

namespace inl::gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class PipelineState : public gxapi::IPipelineState {
public:
	PipelineState(ComPtr<ID3D12PipelineState> native);
	ID3D12PipelineState* GetNative();

private:
	ComPtr<ID3D12PipelineState> m_native;
};

} // namespace inl::gxapi_dx12
