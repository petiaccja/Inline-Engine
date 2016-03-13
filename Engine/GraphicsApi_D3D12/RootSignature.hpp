#pragma once

#include "../GraphicsApi_LL/IRootSignature.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {


class RootSignature : public gxapi::IRootSignature {
public:
	ID3D12RootSignature* GetNative();

protected:
	ID3D12RootSignature* m_native;
};


}
}
