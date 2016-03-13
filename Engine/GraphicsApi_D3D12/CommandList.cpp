#include "CommandList.hpp"

namespace inl {
namespace gxapi {


ID3D12CommandList* CommandList::GetNative()
{
	return m_native;
}


}
}