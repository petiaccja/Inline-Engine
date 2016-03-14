#include "CommandAllocator.hpp"


#include <stdexcept>

namespace inl {
namespace gxapi_dx12 {


CommandAllocator::CommandAllocator(ID3D12CommandAllocator* native) {
	if (native == nullptr) {
		throw std::runtime_error("Null pointer not allowed here.");
	}

	m_native = native;
	//TODO should AddRef be called here?
}


CommandAllocator::~CommandAllocator() {
	m_native->Release();
}


ID3D12CommandAllocator* CommandAllocator::GetNative() {
	return m_native;
}


void CommandAllocator::Reset() {
	m_native->Reset();
}


}
}
