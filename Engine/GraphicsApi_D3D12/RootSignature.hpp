#pragma once

#include "../GraphicsApi_LL/IRootSignature.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class RootSignature : public gxapi::IRootSignature {
public:
	RootSignature(ComPtr<ID3D12RootSignature>& native);

	ID3D12RootSignature* GetNative();

protected:
	ComPtr<ID3D12RootSignature> m_native;
};


} // namespace gxapi_dx12
} // namespace inl
