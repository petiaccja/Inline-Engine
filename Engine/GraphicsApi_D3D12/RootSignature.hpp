#pragma once

#include "../GraphicsApi_LL/IRootSignature.hpp"

#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class RootSignature : public gxapi::IRootSignature {
public:
	ID3D12RootSignature* GetNative();

protected:
	ComPtr<ID3D12RootSignature> m_native;
};


}
}
