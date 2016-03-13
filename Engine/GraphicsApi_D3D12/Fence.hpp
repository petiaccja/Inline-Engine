#pragma once

#include "../GraphicsApi_LL/IFence.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

class Fence : public gxapi::IFence {
public:
	ID3D12Fence* GetNative();

protected:
	ID3D12Fence* m_native;
};

}
}
