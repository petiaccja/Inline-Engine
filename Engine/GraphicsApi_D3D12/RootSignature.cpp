#include "RootSignature.hpp"

namespace inl {
namespace gxapi_dx12 {

RootSignature::RootSignature(ComPtr<ID3D12RootSignature>& native)
	: m_native{native} {
}


ID3D12RootSignature* RootSignature::GetNative() {
	return m_native.Get();
}


} // namespace gxapi_dx12
} // namespace inl
