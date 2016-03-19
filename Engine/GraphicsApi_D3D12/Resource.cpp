#include "Resource.hpp"

#include "NativeCast.hpp"

namespace inl {
namespace gxapi_dx12 {

Resource::Resource(ComPtr<ID3D12Resource>& native)
	: m_native{native} {
}

ID3D12Resource* Resource::GetNative() {
	return m_native.Get();
}


gxapi::ResourceDesc Resource::GetDesc() {
	return native_cast(m_native->GetDesc());
}


} // namespace gxapi_dx12
} // namespace inl
