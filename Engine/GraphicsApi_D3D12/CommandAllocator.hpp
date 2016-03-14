#pragma once

#include "../GraphicsApi_LL/ICommandAllocator.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {


class CommandAllocator : public gxapi::ICommandAllocator {
public:
	CommandAllocator(ID3D12CommandAllocator* native);
	~CommandAllocator();
	CommandAllocator(const CommandAllocator&) = delete;
	CommandAllocator& operator=(const CommandAllocator&) = delete;

	ID3D12CommandAllocator* GetNative();

	virtual void Reset() override;

protected:
	ID3D12CommandAllocator* m_native;
};

}
}
