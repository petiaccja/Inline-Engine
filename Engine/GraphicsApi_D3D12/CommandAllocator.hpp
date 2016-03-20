#pragma once

#include "../GraphicsApi_LL/ICommandAllocator.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>
#include "DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class CommandAllocator : public gxapi::ICommandAllocator {
public:
	CommandAllocator(ComPtr<ID3D12CommandAllocator>& native);
	CommandAllocator(const CommandAllocator&) = delete;
	CommandAllocator& operator=(const CommandAllocator&) = delete;

	ID3D12CommandAllocator* GetNative();

	void Reset() override;

protected:
	ComPtr<ID3D12CommandAllocator> m_native;
};


} // namespace gxapi_dx12
} // namespace inl
