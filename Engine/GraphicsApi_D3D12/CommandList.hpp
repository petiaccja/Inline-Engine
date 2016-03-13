#pragma once

#include "../GraphicsApi_LL/ICommandList.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi {

class CommandList : public ICommandList
{
public:
	
	ID3D12CommandList* GetNative();

protected:
	ID3D12CommandList* m_native;
};

}
}