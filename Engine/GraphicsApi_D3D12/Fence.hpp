#pragma once

#include "../GraphicsApi_LL/IFence.hpp"

#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class Fence : public gxapi::IFence {
public:
	ID3D12Fence* GetNative();

protected:
	ComPtr<ID3D12Fence> m_native;
};

}
}
