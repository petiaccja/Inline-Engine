#include "Fence.hpp"

#include "ExceptionExpansions.hpp"
#include "../GraphicsApi_LL/Exception.hpp"

namespace inl {
namespace gxapi_dx12 {


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
	//static_assert(false, "This is simply wrong, because thread_local is not per-class.");
	// Wrapper for a Win32 event w/ lifetime management
	struct EventHelper {
		EventHelper() {
			evt = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (evt == NULL) {
				throw inl::gxapi::Exception("Failed to create internal Win32 event.");
			}
		}
		~EventHelper() {
			CloseHandle(evt);
		}
		HANDLE evt;
	};

	// Helper object, one HANDLE for every thread
	thread_local EventHelper eventHelper;

	// Wait for value
	HANDLE evt = eventHelper.evt;
	ThrowIfFailed(m_native->SetEventOnCompletion(value, evt));
	WaitForSingleObject(evt, timeoutMillis >= INFINITE ? INFINITE : DWORD(timeoutMillis));	
}


} // namespace gxapi_dx12
} // namespace inl
