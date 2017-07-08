#pragma once

#include "../GraphicsApi_LL/IFence.hpp"
#include "../GraphicsApi_LL/Exception.hpp"
#include "ExceptionExpansions.hpp"

#include <utility>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class Fence : public gxapi::IFence {
	// Wrapper for a Win32 event w/ lifetime management
	struct EventHelper {
		EventHelper() {
			evt = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (evt == NULL) {
				throw RuntimeException("Failed to create internal Win32 event.");
			}
		}
		~EventHelper() {
			if (evt != INVALID_HANDLE_VALUE) {
				CloseHandle(evt);
			}
		}
		EventHelper(const EventHelper& rhs) = delete;
		EventHelper(EventHelper&& rhs) { evt = rhs.evt; rhs.evt = INVALID_HANDLE_VALUE; }
		EventHelper& operator=(const EventHelper& rhs) = delete;
		EventHelper& operator=(EventHelper&& rhs) { evt = rhs.evt; rhs.evt = INVALID_HANDLE_VALUE; return *this; }

		HANDLE evt;
	};
public:
	Fence(ComPtr<ID3D12Fence>& native);
	Fence(const Fence&) = delete;
	Fence& operator=(Fence&) = delete;

	ID3D12Fence* GetNative();

	uint64_t Fetch() const override;
	void Signal(uint64_t value) override;
	void Wait(uint64_t value, uint64_t timeoutMillis = FOREVER) const override;
	void WaitAny(const IFence** fences, uint64_t* values, size_t count, uint64_t timeoutMillis = FOREVER) const override;
	void WaitAll(const IFence** fences, uint64_t* values, size_t count, uint64_t timeoutMillis = FOREVER) const override;
private:
	void WaitMultiple(const IFence** fences, uint64_t* values, size_t count, uint64_t timeoutMillis, bool all) const;
protected:
	ComPtr<ID3D12Fence> m_native;
	static thread_local EventHelper threadEvent;
};


} // namespace gxapi_dx12
} // namespace inl
