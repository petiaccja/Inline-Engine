#pragma once

#include "PipelineState.hpp"
#include "Resource.hpp"

namespace inl {
namespace gxapi_dx12 {


ID3D12PipelineState* native_cast(gxapi::IPipelineState* const source) {
	return static_cast<PipelineState*>(source)->GetNative();
}


ID3D12Resource* native_cast(gxapi::IResource* const source) {
	return static_cast<Resource*>(source)->GetNative();
}


DXGI_FORMAT native_cast(eFormat source) {
	static_assert(false, "TODO");
}


}
}
