#pragma once

#include "../GraphicsApi_LL/ICommandQueue.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"


namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class CommandQueue : public gxapi::ICommandQueue {
public:
	CommandQueue(ComPtr<ID3D12CommandQueue>& native);
	CommandQueue(const CommandQueue&) = delete;
	CommandQueue& operator=(const CommandQueue&) = delete;

	ID3D12CommandQueue* GetNative();

	void ExecuteCommandLists(uint32_t numCommandLists, gxapi::ICommandList* const* commandLists) override;

	void Signal(gxapi::IFence* fence, uint64_t value) override;
	void Wait(gxapi::IFence* fence, uint64_t value) override;

	gxapi::CommandQueueDesc GetDesc() const override;

	void BeginDebuggerEvent(const std::string& name) const override;
	void EndDebuggerEvent() const override;

private:
	ComPtr<ID3D12CommandQueue> m_native;
};


} // namespace gxapi_dx12
} // namespace inl
