#pragma once

#include "../GraphicsApi_LL/ICommandAllocator.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {


class CommandAllocator : public gxapi::ICommandAllocator {
public:

	ID3D12CommandAllocator* GetNative();

protected:
	ID3D12CommandAllocator* m_native;
};

}
}
