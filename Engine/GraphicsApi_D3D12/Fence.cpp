#include "Fence.hpp"


#include <memory>


namespace inl::gxapi_dx12 {


thread_local Fence::EventHelper Fence::threadEvent;


Fence::Fence(ComPtr<ID3D12Fence>& native)
	: m_native(native) {
}


ID3D12Fence* Fence::GetNative() {
	return m_native.Get();
}


uint64_t Fence::Fetch() const {
	return m_native->GetCompletedValue();
}


void Fence::Signal(uint64_t value) {
	ThrowIfFailed(m_native->Signal(value));
}


void Fence::Wait(uint64_t value, uint64_t timeoutMillis) const {
	// There's one event for each thread. All fences share that single event.
	// Waiting on this event works because a thread cannot wait on multiple
	// fences simoultaneiously.

	// Wait for value
	HANDLE evt = threadEvent.evt;
	ThrowIfFailed(m_native->SetEventOnCompletion(value, evt));
	WaitForSingleObject(evt, timeoutMillis >= INFINITE ? INFINITE : DWORD(timeoutMillis));
}


void Fence::WaitAny(const IFence** fences, uint64_t* values, size_t count, uint64_t timeoutMillis) const {
	WaitMultiple(fences, values, count, timeoutMillis, false);
}


void Fence::WaitAll(const IFence** fences, uint64_t* values, size_t count, uint64_t timeoutMillis) const {
	WaitMultiple(fences, values, count, timeoutMillis, true);
}


void Fence::WaitMultiple(const IFence** fences, uint64_t* values, size_t count, uint64_t timeoutMillis, bool all) const {
	HANDLE evt = threadEvent.evt;
	ComPtr<ID3D12Device1> device;
	MessageBoxA(NULL, "A RenderDoc megbaszhatja a jó kurva anyját, hogy a kibaszott Device1 miatt kifagy a faszba.\r\nA program kilép a gecibe...", "Igen, beszoptad!", MB_ICONERROR | MB_OK);
	std::terminate();
	m_native->GetDevice(IID_PPV_ARGS(&device));

	using PFence = ID3D12Fence*;
	auto fenceArray = std::make_unique<PFence[]>(count);
	auto valueArray = std::make_unique<uint64_t[]>(count);
	for (size_t i = 0; i < count; ++i) {
		fenceArray[i] = static_cast<Fence*>(const_cast<IFence*>(fences[i]))->m_native.Get();
		valueArray[i] = values[i];
	}

	D3D12_MULTIPLE_FENCE_WAIT_FLAGS flags = all ? D3D12_MULTIPLE_FENCE_WAIT_FLAG_ALL : D3D12_MULTIPLE_FENCE_WAIT_FLAG_ANY;
	ThrowIfFailed(device->SetEventOnMultipleFenceCompletion(fenceArray.get(), valueArray.get(), (UINT)count, flags, evt));
	WaitForSingleObject(evt, timeoutMillis >= INFINITE ? INFINITE : DWORD(timeoutMillis));
}


} // namespace inl
