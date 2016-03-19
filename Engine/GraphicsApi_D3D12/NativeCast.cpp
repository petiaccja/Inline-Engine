#include "NativeCast.hpp"

#include "../GraphicsApi_LL/Common.hpp"

#include <cassert>

//Dont even think about returning a pointer that points to a locally allocated space
//#include <vector>

namespace inl {
namespace gxapi_dx12 {


////////////////////////////////////////////////////////////
// TO NATIVE
////////////////////////////////////////////////////////////

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


D3D12_SHADER_VISIBILITY native_cast(gxapi::eShaderVisiblity source) {
	switch (source) {
	case inl::gxapi::eShaderVisiblity::ALL:
		return D3D12_SHADER_VISIBILITY_ALL;
	case inl::gxapi::eShaderVisiblity::VERTEX:
		return D3D12_SHADER_VISIBILITY_VERTEX;
	case inl::gxapi::eShaderVisiblity::GEOMETRY:
		return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case inl::gxapi::eShaderVisiblity::HULL:
		return D3D12_SHADER_VISIBILITY_HULL;
	case inl::gxapi::eShaderVisiblity::DOMAIN:
		return D3D12_SHADER_VISIBILITY_DOMAIN;
	case inl::gxapi::eShaderVisiblity::PIXEL:
		return D3D12_SHADER_VISIBILITY_PIXEL;
	default:
		assert(false);
		break;
	}

	return D3D12_SHADER_VISIBILITY{};
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


D3D12_COMMAND_LIST_TYPE native_cast(gxapi::eCommandListType source) {
	switch (source) {
	case gxapi::eCommandListType::COPY:
		return D3D12_COMMAND_LIST_TYPE_COPY;
	case gxapi::eCommandListType::COMPUTE:
		return D3D12_COMMAND_LIST_TYPE_COMPUTE;
	case gxapi::eCommandListType::GRAPHICS:
		return D3D12_COMMAND_LIST_TYPE_DIRECT;
	default:
		assert(false);
	}

	return D3D12_COMMAND_LIST_TYPE{};
}


D3D12_DESCRIPTOR_HEAP_TYPE native_cast(gxapi::eDesriptorHeapType source) {
	using gxapi::eDesriptorHeapType;
	switch (source) {
	case eDesriptorHeapType::CBV_SRV_UAV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	case eDesriptorHeapType::SAMPLER:
		return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	case eDesriptorHeapType::RTV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	case eDesriptorHeapType::DSV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	default:
		assert(false);
	}

	return D3D12_DESCRIPTOR_HEAP_TYPE{};
}


D3D12_ROOT_PARAMETER_TYPE native_cast(gxapi::RootParameterDesc::eType source) {
	switch (source) {
	case gxapi::RootParameterDesc::CONSTANT:
		return D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		break;
	case gxapi::RootParameterDesc::CBV:
		return D3D12_ROOT_PARAMETER_TYPE_CBV;
		break;
	case gxapi::RootParameterDesc::SRV:
		return D3D12_ROOT_PARAMETER_TYPE_SRV;
		break;
	case gxapi::RootParameterDesc::UAV:
		return D3D12_ROOT_PARAMETER_TYPE_UAV;
		break;
	case gxapi::RootParameterDesc::DESCRIPTOR_TABLE:
		return D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		break;
	default:
		assert(false);
		break;
	}

	return D3D12_ROOT_PARAMETER_TYPE{};
}

D3D12_DESCRIPTOR_RANGE_TYPE native_cast(gxapi::DescriptorRange::eType source) {
	switch (source) {
	case inl::gxapi::DescriptorRange::CBV:
		return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	case inl::gxapi::DescriptorRange::SRV:
		return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	case inl::gxapi::DescriptorRange::UAV:
		return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	case inl::gxapi::DescriptorRange::SAMPLER:
		return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	default:
		assert(false);
		break;
	}

	return D3D12_DESCRIPTOR_RANGE_TYPE{};
}


D3D12_TEXTURE_ADDRESS_MODE native_cast(gxapi::eTextureAddressMode source){
	switch (source) {
	case gxapi::eTextureAddressMode::WRAP:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case gxapi::eTextureAddressMode::MIRROR:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case gxapi::eTextureAddressMode::BORDER:
		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	case gxapi::eTextureAddressMode::CLAMP:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case gxapi::eTextureAddressMode::MIRROR_ONE:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	default:
		assert(false);
		break;
	}

	return D3D12_TEXTURE_ADDRESS_MODE{};
}


D3D12_FILTER native_cast(gxapi::eTextureFilterMode source) {
	switch (source) {
	case gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT:
		return D3D12_FILTER_MIN_MAG_MIP_POINT;
	case gxapi::eTextureFilterMode::MIN_MAG_POINT_MIP_LINEAR:
		return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MIN_POINT_MAG_LINEAR_MIP_POINT:
		return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case gxapi::eTextureFilterMode::MIN_POINT_MAG_MIP_LINEAR:
		return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MIN_LINEAR_MAG_MIP_POINT:
		return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	case gxapi::eTextureFilterMode::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MIN_MAG_LINEAR_MIP_POINT:
		return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	case gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR:
		return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	case gxapi::eTextureFilterMode::ANISOTROPIC:
		return D3D12_FILTER_ANISOTROPIC;
	case gxapi::eTextureFilterMode::COMPARISON_MIN_MAG_MIP_POINT:
		return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	case gxapi::eTextureFilterMode::COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
		return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
	case gxapi::eTextureFilterMode::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case gxapi::eTextureFilterMode::COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
		return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
	case gxapi::eTextureFilterMode::COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
		return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
	case gxapi::eTextureFilterMode::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case gxapi::eTextureFilterMode::COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
		return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	case gxapi::eTextureFilterMode::COMPARISON_MIN_MAG_MIP_LINEAR:
		return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	case gxapi::eTextureFilterMode::COMPARISON_ANISOTROPIC:
		return D3D12_FILTER_COMPARISON_ANISOTROPIC;
	case gxapi::eTextureFilterMode::MINIMUM_MIN_MAG_MIP_POINT:
		return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
	case gxapi::eTextureFilterMode::MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
		return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case gxapi::eTextureFilterMode::MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
		return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
		return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
	case gxapi::eTextureFilterMode::MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
		return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
	case gxapi::eTextureFilterMode::MINIMUM_MIN_MAG_MIP_LINEAR:
		return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MINIMUM_ANISOTROPIC:
		return D3D12_FILTER_MINIMUM_ANISOTROPIC;
	case gxapi::eTextureFilterMode::MAXIMUM_MIN_MAG_MIP_POINT:
		return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
	case gxapi::eTextureFilterMode::MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
		return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case gxapi::eTextureFilterMode::MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
		return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
		return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
	case gxapi::eTextureFilterMode::MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
		return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
	case gxapi::eTextureFilterMode::MAXIMUM_MIN_MAG_MIP_LINEAR:
		return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
	case gxapi::eTextureFilterMode::MAXIMUM_ANISOTROPIC:
		return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
	default:
		assert(false);
		break;
	}

	return D3D12_FILTER{};
}


D3D12_COMPARISON_FUNC native_cast(gxapi::eComparisonFunction source) {
	switch (source) {
	case inl::gxapi::eComparisonFunction::NEVER:
		return D3D12_COMPARISON_FUNC_NEVER;
	case inl::gxapi::eComparisonFunction::LESS:
		return D3D12_COMPARISON_FUNC_LESS;
	case inl::gxapi::eComparisonFunction::LESS_EQUAL:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case inl::gxapi::eComparisonFunction::GREATER:
		return D3D12_COMPARISON_FUNC_GREATER;
	case inl::gxapi::eComparisonFunction::GREATER_EQUAL:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case inl::gxapi::eComparisonFunction::EQUAL:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case inl::gxapi::eComparisonFunction::NOT_EQUAL:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case inl::gxapi::eComparisonFunction::ALWAYS:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	default:
		assert(false);
		break;
	}

	return D3D12_COMPARISON_FUNC{};
}


INT native_cast(gxapi::eCommandQueuePriority source) {
	switch (source) {
	case gxapi::eCommandQueuePriority::NORMAL:
		return D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	case gxapi::eCommandQueuePriority::HIGH:
		return D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	default:
		assert(false);
	}

	return 0;
}


D3D12_STATIC_BORDER_COLOR native_cast(gxapi::eTextureBorderColor source) {
	switch (source) {
	case inl::gxapi::eTextureBorderColor::TRANSPARENT_BLACK:
		return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	case inl::gxapi::eTextureBorderColor::OPAQUE_BLACK:
		return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	case inl::gxapi::eTextureBorderColor::OPAQUE_WHITE:
		return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	default:
		assert(false);
		break;
	}

	return D3D12_STATIC_BORDER_COLOR{};
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


D3D12_BOX native_cast(gxapi::Cube source) {
	D3D12_BOX result;
	result.back = source.back;
	result.front = source.front;
	result.left = source.left;
	result.right = source.right;
	result.bottom = source.bottom;
	result.top = source.top;

	return result;
}


DXGI_FORMAT native_cast(gxapi::eFormat source) {
	static_assert(false, "TODO");

	return DXGI_FORMAT{};
}


D3D12_DESCRIPTOR_RANGE native_cast(gxapi::DescriptorRange source) {
	D3D12_DESCRIPTOR_RANGE result;

	result.BaseShaderRegister = source.baseShaderRegister;
	result.NumDescriptors = source.numDescriptors;
	result.OffsetInDescriptorsFromTableStart = source.offsetFromTableStart;
	result.RangeType = native_cast(source.type);
	result.RegisterSpace = source.registerSpace;

	return result;
}

D3D12_ROOT_CONSTANTS native_cast(gxapi::RootConstant source) {
	D3D12_ROOT_CONSTANTS result;

	result.Num32BitValues = source.numConstants;
	result.RegisterSpace = source.registerSpace;
	result.ShaderRegister = source.shaderRegister;

	return result;
}


D3D12_ROOT_DESCRIPTOR native_cast(gxapi::RootDescriptor source) {
	D3D12_ROOT_DESCRIPTOR result;

	result.RegisterSpace = source.registerSpace;
	result.ShaderRegister = source.shaderRegister;

	return result;
}


D3D12_HEAP_PROPERTIES native_cast(gxapi::HeapProperties source) {
	D3D12_HEAP_PROPERTIES result;
	

	result.CPUPageProperty = native_cast(source.cpuPageProperty);

	result.MemoryPoolPreference = source.pool;
	static_assert(false, "TODO");
	result.Type = native_cast(source.type);
	result.CreationNodeMask = 0;
	result.VisibleNodeMask = 0;

	return result;
}


D3D12_STATIC_SAMPLER_DESC native_cast(gxapi::StaticSamplerDesc source) {
	D3D12_STATIC_SAMPLER_DESC result;

	result.AddressU = native_cast(source.addressU);
	result.AddressV = native_cast(source.addressV);
	result.AddressW = native_cast(source.addressW);
	result.BorderColor = native_cast(source.border);
	result.ComparisonFunc = native_cast(source.compareFunc);
	result.Filter = native_cast(source.filter);
	result.MaxAnisotropy = source.maxAnisotropy;
	result.MaxLOD = source.maxMipLevel;
	result.MinLOD = source.minMipLevel;
	result.MipLODBias = source.mipLevelBias;
	result.RegisterSpace = source.registerSpace;
	result.ShaderRegister = source.shaderRegister;
	result.ShaderVisibility = native_cast(source.shaderVisibility);

	return result;
}


D3D12_COMMAND_QUEUE_DESC native_cast(gxapi::CommandQueueDesc source) {
	D3D12_COMMAND_QUEUE_DESC result;
	result.Type = native_cast(source.type);
	result.Flags =  (source.enableGpuTimeout==false) ? D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT : D3D12_COMMAND_QUEUE_FLAG_NONE;
	result.Priority = native_cast(source.priority);
	result.NodeMask = 0;

	return result;
}


D3D12_DESCRIPTOR_HEAP_DESC native_cast(gxapi::DescriptorHeapDesc source) {
	D3D12_DESCRIPTOR_HEAP_DESC result;

	result.Type = native_cast(source.type);
	result.NumDescriptors = source.numDescriptors;
	result.Flags = source.isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result.NodeMask = 0;

	return result;
}








////////////////////////////////////////////////////////////
// FROM NATIVE
////////////////////////////////////////////////////////////


gxapi::eTextueDimension texture_dimension_cast(D3D12_RESOURCE_DIMENSION source) {
	using gxapi::eTextueDimension;

	switch (source) {
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		return eTextueDimension::ONE;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		return eTextueDimension::TWO;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		return eTextueDimension::THREE;
	default:
		assert(false); //asserts on UKNOWN, and BUFFER too
	}

	return gxapi::eTextueDimension{};
}


gxapi::eFormat native_cast(DXGI_FORMAT source) {
	static_assert(false, "TODO");
	return gxapi::eFormat{};
}


gxapi::eTextureLayout native_cast(D3D12_TEXTURE_LAYOUT source) {
	using gxapi::eTextureLayout;
	switch (source) {
	case D3D12_TEXTURE_LAYOUT_UNKNOWN:
		return eTextureLayout::UNKNOWN;
	case D3D12_TEXTURE_LAYOUT_ROW_MAJOR:
		return eTextureLayout::ROW_MAJOR;
	case D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE:
		return eTextureLayout::UNDEFINED_SWIZZLE;
	case D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE:
		return eTextureLayout::STANDARD_SWIZZLE;
	default:
		assert(false);
	}

	return gxapi::eTextureLayout{};
}


gxapi::eResourceFlags native_cast(D3D12_RESOURCE_FLAGS source) {
	using gxapi::eResourceFlags;

	gxapi::eResourceFlags result = eResourceFlags::NONE;

	if ((source & D3D12_RESOURCE_FLAG_NONE) != 0) {
		result |= eResourceFlags::NONE;
	}
	if ((source & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0) {
		result |= eResourceFlags::ALLOW_RENDER_TARGET;
	}
	if ((source & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0) {
		result |= eResourceFlags::ALLOW_DEPTH_STENCIL;
	}
	if ((source & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0) {
		result |= eResourceFlags::ALLOW_UNORDERED_ACCESS;
	}
	if ((source & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) != 0) {
		result |= eResourceFlags::DENY_SHADER_RESOURCE;
	}
	if ((source & D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER) != 0) {
		result |= eResourceFlags::ALLOW_CROSS_ADAPTER;
	}
	if ((source & D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS) != 0) {
		result |= eResourceFlags::ALLOW_SIMULTANEOUS_ACCESS;
	}

	return result;
}


gxapi::eCommandListType native_cast(D3D12_COMMAND_LIST_TYPE source) {
	switch (source) {
	case D3D12_COMMAND_LIST_TYPE_BUNDLE:
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return gxapi::eCommandListType::GRAPHICS;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return gxapi::eCommandListType::COMPUTE;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return gxapi::eCommandListType::COPY;
	default:
		assert(false);
	}

	return gxapi::eCommandListType{};
}


gxapi::eCommandQueuePriority native_cast(D3D12_COMMAND_QUEUE_PRIORITY source) {
	switch (source) {
	case D3D12_COMMAND_QUEUE_PRIORITY_NORMAL:
		return gxapi::eCommandQueuePriority::NORMAL;
	case D3D12_COMMAND_QUEUE_PRIORITY_HIGH:
		return gxapi::eCommandQueuePriority::HIGH;
	default:
		assert(false);
	}

	return gxapi::eCommandQueuePriority{};
}

gxapi::eDesriptorHeapType native_cast(D3D12_DESCRIPTOR_HEAP_TYPE source) {
	using gxapi::eDesriptorHeapType;

	switch (source) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return eDesriptorHeapType::CBV_SRV_UAV;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return eDesriptorHeapType::SAMPLER;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return eDesriptorHeapType::RTV;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return eDesriptorHeapType::DSV;
	default:
		assert(false);
	}

	return eDesriptorHeapType{};
}


gxapi::CommandQueueDesc native_cast(D3D12_COMMAND_QUEUE_DESC source) {
	gxapi::CommandQueueDesc result;

	bool disableGpuTimeout = (source.Flags & D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT) != 0;
	result.enableGpuTimeout = !disableGpuTimeout;
	result.priority = native_cast(static_cast<D3D12_COMMAND_QUEUE_PRIORITY>(source.Priority));
	result.type = native_cast(source.Type);

	return result;
}


gxapi::DescriptorHeapDesc native_cast(D3D12_DESCRIPTOR_HEAP_DESC source) {
	gxapi::DescriptorHeapDesc result;

	result.isShaderVisible = (source.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) != 0;
	result.numDescriptors = source.NumDescriptors;
	result.type = native_cast(source.Type);

	return result;
}


gxapi::ResourceDesc native_cast(D3D12_RESOURCE_DESC source) {
	gxapi::ResourceDesc result;

	bool typeUnknown = source.Dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN;
	assert(typeUnknown == false);

	result.type = (source.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? gxapi::eResourceType::BUFFER : gxapi::eResourceType::TEXTURE;

	if (result.type == gxapi::eResourceType::BUFFER) {
		gxapi::BufferDesc bufferDesc;

		bufferDesc.sizeInBytes = source.Width;

		result.bufferDesc = bufferDesc;
	}
	else {
		result.textureDesc = gxapi::TextureDesc{
			texture_dimension_cast(source.Dimension),
			source.Alignment,
			source.Width,
			source.Height,
			source.DepthOrArraySize,
			source.MipLevels,
			native_cast(source.Format),
			native_cast(source.Layout),
			native_cast(source.Flags),
			source.SampleDesc.Count,
			source.SampleDesc.Quality
		};
	}

	return result;
}



} // namespace gxapi_dx12
} // namespace inl
