#include "PipelineState.hpp"

namespace inl {
namespace gxapi_dx12 {


ID3D12PipelineState* PipelineState::GetNative() {
	return m_native;
}


}
}
