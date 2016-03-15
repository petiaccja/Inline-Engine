#include "NativeCast.hpp"

#include "../GraphicsApi_LL/Common.hpp"

#include <cassert>

namespace inl {
namespace gxapi_dx12 {


ID3D12PipelineState* native_cast(gxapi::IPipelineState* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<PipelineState*>(source)->GetNative();
}


ID3D12Resource* native_cast(gxapi::IResource* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<Resource*>(source)->GetNative();
}


ID3D12CommandAllocator* native_cast(gxapi::ICommandAllocator* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<CommandAllocator*>(source)->GetNative();
}


ID3D12GraphicsCommandList* native_cast(gxapi::IGraphicsCommandList* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<GraphicsCommandList*>(source)->GetNative();
}


ID3D12RootSignature* native_cast(gxapi::IRootSignature* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<RootSignature*>(source)->GetNative();
}


ID3D12Fence* native_cast(gxapi::IFence * source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<Fence*>(source)->GetNative();
}


D3D12_PRIMITIVE_TOPOLOGY native_cast(gxapi::ePrimitiveTopology source) {
	using gxapi::ePrimitiveTopology;

	switch (source) {
	case ePrimitiveTopology::LINELIST:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
	case ePrimitiveTopology::LINESTRIP:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case ePrimitiveTopology::POINTLIST:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;
	case ePrimitiveTopology::TRIANGLELIST:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case ePrimitiveTopology::TRIANGLESTRIP:
		return D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	default:
		assert(false);
	}

	return D3D12_PRIMITIVE_TOPOLOGY{};
}


D3D12_VIEWPORT native_cast(gxapi::Viewport const & source) {
	D3D12_VIEWPORT result;

	result.TopLeftX = source.topLeftX;
	result.TopLeftY = source.topLeftY;
	result.Height = source.height;
	result.Width = source.width;
	result.MinDepth = source.minDepth;
	result.MaxDepth = source.maxDepth;

	return result;
}


D3D12_RECT native_cast(gxapi::Rectangle const & source) {
	D3D12_RECT result;

	result.left = source.left;
	result.right = source.right;
	result.bottom = source.bottom;
	result.top = source.top;

	return result;
}


DXGI_FORMAT native_cast(gxapi::eFormat source) {
	//TODO
	assert(false);

	return DXGI_FORMAT{};
}


} // namespace gxapi_dx12
} // namespace inl
