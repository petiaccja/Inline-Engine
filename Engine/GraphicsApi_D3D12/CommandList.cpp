#include "CommandList.hpp"

namespace inl {
namespace gxapi_dx12 {


ID3D12CommandList* CommandList::GetNative() {
	return m_native;
}


}
}