#pragma once
#include "../GraphicsApi_LL/IFence.hpp"
#include <memory>
#include <cstdint>


namespace inl {
namespace gxeng {


class SyncPoint {
public:
	SyncPoint(std::shared_ptr<gxapi::IFence> fence, uint64_t value) :
		m_fence(fence), m_value(value) {}
	void Complete() {

	}
	void Wait();
private:
	std::shared_ptr<gxapi::IFence> m_fence;
	uint64_t m_value;
};



}
}