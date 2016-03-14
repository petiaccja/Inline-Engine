#include "Resource.hpp"

namespace inl {
namespace gxapi_dx12 {


ID3D12Resource* Resource::GetNative() {
	return m_native.Get();
}


}
}
