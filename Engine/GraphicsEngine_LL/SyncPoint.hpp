#pragma once

#include <GraphicsApi_LL/IFence.hpp>
#include <cstdint>
#include <cassert>
#include <memory>


namespace inl::gxeng {


class CommandQueue;
class ResourceResidencyQueue;



class SyncPoint {
	friend class inl::gxeng::CommandQueue;
	friend class inl::gxeng::ResourceResidencyQueue;
public:
	SyncPoint() : m_value(0) {}
	SyncPoint(std::shared_ptr<gxapi::IFence> fence, uint64_t value)
		: m_fence(fence), m_value(value)
	{}

	void Wait() const {
		assert((bool)m_fence);
		m_fence->Wait(m_value);		
	}

	operator bool() {
		return (bool)m_fence;
	}
private:
	std::shared_ptr<gxapi::IFence> m_fence;
	uint64_t m_value;
};



} // namespace gxeng