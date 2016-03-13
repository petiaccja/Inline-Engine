#pragma once

#include "../GraphicsApi_LL/IFence.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi {

class Fence : public IFence
{
public:
	ID3D12Fence* GetNative();

protected:
	ID3D12Fence* m_native;
};

}
}
