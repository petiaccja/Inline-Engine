#pragma once

#include "PipelineState.hpp"


namespace inl {
namespace gxapi_dx12 {


template <typename SourceType>
void native_cast(SourceType const& source) {
	static_assert(false, "SourceType not supprted!");
}


ID3D12PipelineState* native_cast(gxapi::IPipelineState* const source) {
	return static_cast<PipelineState*>(source)->GetNative();
}


D3D12_CPU_DESCRIPTOR_HANDLE native_cast(gxapi::DescriptorHandle const& source) {
	D3D12_CPU_DESCRIPTOR_HANDLE result;
	result;
	//source.

	return ;
}



}
}
