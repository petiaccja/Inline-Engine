#pragma once

#include "../GraphicsApi_LL/IFence.hpp"
#include "SyncPoint.hpp"
#include <memory>
#include <atomic>

namespace inl {
namespace gxeng {


class Timeline {
public:
	Timeline(gxapi::IFence* fence) :
		m_fence(fence), m_value(0) {}

	SyncPoint CreateSyncPoint() {
		uint64_t syncPointValue = m_value++;
		return SyncPoint(m_fence, syncPointValue);
	}
private:
	std::shared_ptr<gxapi::IFence> m_fence;
	std::atomic_uint64_t m_value;
};


} // namespace gxeng
} // namespace inl