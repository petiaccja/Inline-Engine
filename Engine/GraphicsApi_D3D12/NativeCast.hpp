#pragma once

#include "PipelineState.hpp"
#include "Resource.hpp"
#include "CommandAllocator.hpp"

namespace inl {
namespace gxapi_dx12 {


ID3D12PipelineState* native_cast(gxapi::IPipelineState* source) {
	return static_cast<PipelineState*>(source)->GetNative();
}


ID3D12Resource* native_cast(gxapi::IResource* source) {
	return static_cast<Resource*>(source)->GetNative();
}


ID3D12CommandAllocator* native_cast(gxapi::ICommandAllocator* source) {
	return static_cast<CommandAllocator*>(source)->GetNative();
}


DXGI_FORMAT native_cast(eFormat source) {
	static_assert(false, "TODO");
}


}
}
