#pragma once

#include "../GraphicsApi_LL/IFence.hpp"

#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class Fence : public gxapi::IFence {
public:
	Fence(ComPtr<ID3D12Fence>& native);
	Fence(const Fence&) = delete;
	Fence& operator=(Fence&) = delete;

	ID3D12Fence* GetNative();

	uint64_t Fetch() override;
	void Signal(uint64_t value) override;

protected:
	ComPtr<ID3D12Fence> m_native;
};


} // namespace gxapi_dx12
} // namespace inl
