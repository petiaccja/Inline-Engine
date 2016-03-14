#include "RootSignature.hpp"

namespace inl {
namespace gxapi_dx12 {


ID3D12RootSignature* RootSignature::GetNative() {
	return m_native.Get();
}


}
}

