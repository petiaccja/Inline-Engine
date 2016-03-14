#include "Fence.hpp"

namespace inl {
namespace gxapi_dx12 {


ID3D12Fence* Fence::GetNative() {
	return m_native.Get();
}


} // namespace gxapi_dx12
} // namespace inl
