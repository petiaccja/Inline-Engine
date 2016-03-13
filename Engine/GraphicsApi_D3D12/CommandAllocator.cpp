#include "CommandAllocator.hpp"


namespace inl {
namespace gxapi_dx12 {


ID3D12CommandAllocator* CommandAllocator::GetNative()
{
	return m_native;
}


}
}
