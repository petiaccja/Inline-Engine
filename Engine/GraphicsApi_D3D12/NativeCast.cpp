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

//---------------
//INTERFACE

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


//---------------
//ENUM

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


D3D12_HEAP_TYPE native_cast(gxapi::eHeapType source) {
	switch (source) {
	case inl::gxapi::eHeapType::DEFAULT:
		return D3D12_HEAP_TYPE_DEFAULT;
	case inl::gxapi::eHeapType::UPLOAD:
		return D3D12_HEAP_TYPE_UPLOAD;
	case inl::gxapi::eHeapType::READBACK:
		return D3D12_HEAP_TYPE_READBACK;
	case inl::gxapi::eHeapType::CUSTOM:
		return D3D12_HEAP_TYPE_CUSTOM;
	default:
		assert(false);
		break;
	}

	return D3D12_HEAP_TYPE{};
}


D3D12_CPU_PAGE_PROPERTY native_cast(gxapi::eCpuPageProperty source) {
	switch (source) {
	case inl::gxapi::eCpuPageProperty::UNKNOWN:
		return D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	case inl::gxapi::eCpuPageProperty::NOT_AVAILABLE:
		return D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
	case inl::gxapi::eCpuPageProperty::WRITE_COMBINE:
		return D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
	case inl::gxapi::eCpuPageProperty::WRITE_BACK:
		return D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	default:
		assert(false);
		break;
	}

	return D3D12_CPU_PAGE_PROPERTY{};
}


D3D12_MEMORY_POOL native_cast(gxapi::eMemoryPool source) {
	switch (source) {
	case inl::gxapi::eMemoryPool::UNKNOWN:
		return D3D12_MEMORY_POOL_UNKNOWN;
	case inl::gxapi::eMemoryPool::HOST:
		return D3D12_MEMORY_POOL_L0;
	case inl::gxapi::eMemoryPool::DEDICATED:
		return D3D12_MEMORY_POOL_L1;
	default:
		assert(false);
		break;
	}

	return D3D12_MEMORY_POOL{};
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


DXGI_FORMAT native_cast(gxapi::eFormat source) {
	switch (source) {
	case inl::gxapi::eFormat::UNKNOWN:
		return DXGI_FORMAT_UNKNOWN;
	case inl::gxapi::eFormat::R32G32B32A32_TYPELESS:
		return DXGI_FORMAT_R32G32B32A32_TYPELESS;
	case inl::gxapi::eFormat::R32G32B32A32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case inl::gxapi::eFormat::R32G32B32A32_UINT:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case inl::gxapi::eFormat::R32G32B32A32_SINT:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case inl::gxapi::eFormat::R32G32B32_TYPELESS:
		return DXGI_FORMAT_R32G32B32_TYPELESS;
	case inl::gxapi::eFormat::R32G32B32_FLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case inl::gxapi::eFormat::R32G32B32_UINT:
		return DXGI_FORMAT_R32G32B32_UINT;
	case inl::gxapi::eFormat::R32G32B32_SINT:
		return DXGI_FORMAT_R32G32B32_SINT;
	case inl::gxapi::eFormat::R16G16B16A16_TYPELESS:
		return DXGI_FORMAT_R16G16B16A16_TYPELESS;
	case inl::gxapi::eFormat::R16G16B16A16_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case inl::gxapi::eFormat::R16G16B16A16_UNORM:
		return DXGI_FORMAT_R16G16B16A16_UNORM;
	case inl::gxapi::eFormat::R16G16B16A16_UINT:
		return DXGI_FORMAT_R16G16B16A16_UINT;
	case inl::gxapi::eFormat::R16G16B16A16_SNORM:
		return DXGI_FORMAT_R16G16B16A16_SNORM;
	case inl::gxapi::eFormat::R16G16B16A16_SINT:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case inl::gxapi::eFormat::R32G32_TYPELESS:
		return DXGI_FORMAT_R32G32_TYPELESS;
	case inl::gxapi::eFormat::R32G32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
	case inl::gxapi::eFormat::R32G32_UINT:
		return DXGI_FORMAT_R32G32_UINT;
	case inl::gxapi::eFormat::R32G32_SINT:
		return DXGI_FORMAT_R32G32_SINT;
	case inl::gxapi::eFormat::R32G8X24_TYPELESS:
		return DXGI_FORMAT_R32G8X24_TYPELESS;
	case inl::gxapi::eFormat::D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case inl::gxapi::eFormat::R32_FLOAT_X8X24_TYPELESS:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	case inl::gxapi::eFormat::X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
	case inl::gxapi::eFormat::R10G10B10A2_TYPELESS:
		return DXGI_FORMAT_R10G10B10A2_TYPELESS;
	case inl::gxapi::eFormat::R10G10B10A2_UNORM:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case inl::gxapi::eFormat::R10G10B10A2_UINT:
		return DXGI_FORMAT_R10G10B10A2_UINT;
	case inl::gxapi::eFormat::R11G11B10_FLOAT:
		return DXGI_FORMAT_R11G11B10_FLOAT;
	case inl::gxapi::eFormat::R8G8B8A8_TYPELESS:
		return DXGI_FORMAT_R8G8B8A8_TYPELESS;
	case inl::gxapi::eFormat::R8G8B8A8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case inl::gxapi::eFormat::R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case inl::gxapi::eFormat::R8G8B8A8_UINT:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case inl::gxapi::eFormat::R8G8B8A8_SNORM:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case inl::gxapi::eFormat::R8G8B8A8_SINT:
		return DXGI_FORMAT_R8G8B8A8_SINT;
	case inl::gxapi::eFormat::R16G16_TYPELESS:
		return DXGI_FORMAT_R16G16_TYPELESS;
	case inl::gxapi::eFormat::R16G16_FLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;
	case inl::gxapi::eFormat::R16G16_UNORM:
		return DXGI_FORMAT_R16G16_UNORM;
	case inl::gxapi::eFormat::R16G16_UINT:
		return DXGI_FORMAT_R16G16_UINT;
	case inl::gxapi::eFormat::R16G16_SNORM:
		return DXGI_FORMAT_R16G16_SNORM;
	case inl::gxapi::eFormat::R16G16_SINT:
		return DXGI_FORMAT_R16G16_SINT;
	case inl::gxapi::eFormat::R32_TYPELESS:
		return DXGI_FORMAT_R32_TYPELESS;
	case inl::gxapi::eFormat::D32_FLOAT:
		return DXGI_FORMAT_D32_FLOAT;
	case inl::gxapi::eFormat::R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case inl::gxapi::eFormat::R32_UINT:
		return DXGI_FORMAT_R32_UINT;
	case inl::gxapi::eFormat::R32_SINT:
		return DXGI_FORMAT_R32_SINT;
	case inl::gxapi::eFormat::R24G8_TYPELESS:
		return DXGI_FORMAT_R24G8_TYPELESS;
	case inl::gxapi::eFormat::D24_UNORM_S8_UINT:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case inl::gxapi::eFormat::R24_UNORM_X8_TYPELESS:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case inl::gxapi::eFormat::X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
	case inl::gxapi::eFormat::R8G8_TYPELESS:
		return DXGI_FORMAT_R8G8_TYPELESS;
	case inl::gxapi::eFormat::R8G8_UNORM:
		return DXGI_FORMAT_R8G8_UNORM;
	case inl::gxapi::eFormat::R8G8_UINT:
		return DXGI_FORMAT_R8G8_UINT;
	case inl::gxapi::eFormat::R8G8_SNORM:
		return DXGI_FORMAT_R8G8_SNORM;
	case inl::gxapi::eFormat::R8G8_SINT:
		return DXGI_FORMAT_R8G8_SINT;
	case inl::gxapi::eFormat::R16_TYPELESS:
		return DXGI_FORMAT_R16_TYPELESS;
	case inl::gxapi::eFormat::R16_FLOAT:
		return DXGI_FORMAT_R16_FLOAT;
	case inl::gxapi::eFormat::D16_UNORM:
		return DXGI_FORMAT_D16_UNORM;
	case inl::gxapi::eFormat::R16_UNORM:
		return DXGI_FORMAT_R16_UNORM;
	case inl::gxapi::eFormat::R16_UINT:
		return DXGI_FORMAT_R16_UINT;
	case inl::gxapi::eFormat::R16_SNORM:
		return DXGI_FORMAT_R16_SNORM;
	case inl::gxapi::eFormat::R16_SINT:
		return DXGI_FORMAT_R16_SINT;
	case inl::gxapi::eFormat::R8_TYPELESS:
		return DXGI_FORMAT_R8_TYPELESS;
	case inl::gxapi::eFormat::R8_UNORM:
		return DXGI_FORMAT_R8_UNORM;
	case inl::gxapi::eFormat::R8_UINT:
		return DXGI_FORMAT_R8_UINT;
	case inl::gxapi::eFormat::R8_SNORM:
		return DXGI_FORMAT_R8_SNORM;
	case inl::gxapi::eFormat::R8_SINT:
		return DXGI_FORMAT_R8_SINT;
	case inl::gxapi::eFormat::A8_UNORM:
		return DXGI_FORMAT_A8_UNORM;

	default:
		assert(false);
		break;
	}

	return DXGI_FORMAT{};
}


D3D12_TEXTURE_LAYOUT native_cast(gxapi::eTextureLayout source) {
	switch (source) {
	case inl::gxapi::eTextureLayout::UNKNOWN:
		return D3D12_TEXTURE_LAYOUT_UNKNOWN;
	case inl::gxapi::eTextureLayout::ROW_MAJOR:
		return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	case inl::gxapi::eTextureLayout::UNDEFINED_SWIZZLE:
		return D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
	case inl::gxapi::eTextureLayout::STANDARD_SWIZZLE:
		return D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE;
	default:
		assert(false);
		break;
	}

	return D3D12_TEXTURE_LAYOUT{};
}


D3D12_SHADER_BYTECODE native_cast(gxapi::ShaderByteCodeDesc source) {
	D3D12_SHADER_BYTECODE result;

	result.BytecodeLength = source.sizeOfByteCode;
	result.pShaderBytecode = source.shaderByteCode;

	return result;
}


D3D12_RESOURCE_DIMENSION native_cast(gxapi::eTextueDimension source) {
	switch (source) {
	case inl::gxapi::eTextueDimension::ONE:
		return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	case inl::gxapi::eTextueDimension::TWO:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case inl::gxapi::eTextueDimension::THREE:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	default:
		assert(false);
		break;
	}

	return D3D12_RESOURCE_DIMENSION{};
}


D3D12_RESOURCE_STATES native_cast(gxapi::eResourceState source) {
	switch (source) {
	case inl::gxapi::eResourceState::COMMON:
		return D3D12_RESOURCE_STATE_COMMON;
	case inl::gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER:
		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case inl::gxapi::eResourceState::INDEX_BUFFER:
		return D3D12_RESOURCE_STATE_INDEX_BUFFER;
	case inl::gxapi::eResourceState::RENDER_TARGET:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case inl::gxapi::eResourceState::UNORDERED_ACCESS:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case inl::gxapi::eResourceState::DEPTH_WRITE:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case inl::gxapi::eResourceState::DEPTH_READ:
		return D3D12_RESOURCE_STATE_DEPTH_READ;
	case inl::gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE:
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case inl::gxapi::eResourceState::PIXEL_SHADER_RESOURCE:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case inl::gxapi::eResourceState::STREAM_OUT:
		return D3D12_RESOURCE_STATE_STREAM_OUT;
	case inl::gxapi::eResourceState::INDIRECT_ARGUMENT:
		return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	case inl::gxapi::eResourceState::COPY_DEST:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case inl::gxapi::eResourceState::COPY_SOURCE:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case inl::gxapi::eResourceState::RESOLVE_DEST:
		return D3D12_RESOURCE_STATE_RESOLVE_DEST;
	case inl::gxapi::eResourceState::RESOLVE_SOURCE:
		return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
	case inl::gxapi::eResourceState::GENERIC_READ:
		return D3D12_RESOURCE_STATE_GENERIC_READ;
	case inl::gxapi::eResourceState::PRESENT:
		return D3D12_RESOURCE_STATE_PRESENT;
	case inl::gxapi::eResourceState::PREDICATION:
		return D3D12_RESOURCE_STATE_PREDICATION;
	default:
		assert(false);
		break;
	}

	return D3D12_RESOURCE_STATES{};
}


D3D12_RESOURCE_FLAGS native_cast(gxapi::eResourceFlags source) {
	D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;

	if ((source & gxapi::eResourceFlags::ALLOW_RENDER_TARGET) != 0) {
		result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if ((source & gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL) != 0) {
		result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	if ((source & gxapi::eResourceFlags::ALLOW_UNORDERED_ACCESS) != 0) {
		result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	if ((source & gxapi::eResourceFlags::DENY_SHADER_RESOURCE) != 0) {
		result |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}
	if ((source & gxapi::eResourceFlags::ALLOW_CROSS_ADAPTER) != 0) {
		result |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
	}
	if ((source & gxapi::eResourceFlags::ALLOW_SIMULTANEOUS_ACCESS) != 0) {
		result |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
	}

	return result;
}


D3D12_HEAP_FLAGS native_cast(gxapi::eHeapFlags source) {
	D3D12_HEAP_FLAGS result = D3D12_HEAP_FLAG_NONE;

	if ((source & inl::gxapi::eHeapFlags::SHARED) != 0) {
		result |= D3D12_HEAP_FLAG_SHARED;
	}
	if ((source & inl::gxapi::eHeapFlags::DENY_BUFFERS) != 0) {
		result |= D3D12_HEAP_FLAG_DENY_BUFFERS;
	}
	if ((source & inl::gxapi::eHeapFlags::ALLOW_DISPLAY) != 0) {
		result |= D3D12_HEAP_FLAG_ALLOW_DISPLAY;
	}
	if ((source & inl::gxapi::eHeapFlags::SHARED_CROSS_ADAPTER) != 0) {
		result |= D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER;
	}
	if ((source & inl::gxapi::eHeapFlags::DENY_RT_DS_TEXTURES) != 0) {
		result |= D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
	}
	if ((source & inl::gxapi::eHeapFlags::DENY_NON_RT_DS_TEXTURES) != 0) {
		result |= D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
	}
	if ((source & inl::gxapi::eHeapFlags::ALLOW_ALL_BUFFERS_AND_TEXTURES) != 0) {
		result |= D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
	}
	if ((source & inl::gxapi::eHeapFlags::ALLOW_ONLY_BUFFERS) != 0) {
		result |= D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
	}
	if ((source & inl::gxapi::eHeapFlags::ALLOW_ONLY_NON_RT_DS_TEXTURES) != 0) {
		result |= D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	}
	if ((source & inl::gxapi::eHeapFlags::ALLOW_ONLY_RT_DS_TEXTURES) != 0) {
		result |= D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
	}

	return result;
}




//---------------
//OBJECT

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


D3D12_CLEAR_VALUE native_cast(gxapi::ClearValue source) {
	D3D12_CLEAR_VALUE result;

	result.Color[0] = source.color.r;
	result.Color[1] = source.color.g;
	result.Color[2] = source.color.b;
	result.Color[3] = source.color.a;

	result.DepthStencil.Depth = source.depthStencil.depth;
	result.DepthStencil.Stencil = source.depthStencil.stencil;

	result.Format = native_cast(source.format);

	return result;
}


//---------------
//DESCRIPTOR

D3D12_ROOT_DESCRIPTOR native_cast(gxapi::RootDescriptor source) {
	D3D12_ROOT_DESCRIPTOR result;

	result.RegisterSpace = source.registerSpace;
	result.ShaderRegister = source.shaderRegister;

	return result;
}


D3D12_HEAP_PROPERTIES native_cast(gxapi::HeapProperties source) {
	D3D12_HEAP_PROPERTIES result;
	
	result.Type = native_cast(source.type);
	result.CPUPageProperty = native_cast(source.cpuPageProperty);
	result.MemoryPoolPreference = native_cast(source.pool);
	result.CreationNodeMask = 0;
	result.VisibleNodeMask = 0;

	return result;
}


D3D12_RESOURCE_DESC native_cast(gxapi::ResourceDesc source) {
	D3D12_RESOURCE_DESC result;

	if (source.type == gxapi::eResourceType::BUFFER) {
		result.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		result.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		result.Width = source.bufferDesc.sizeInBytes;
		result.Height = 1;
		result.DepthOrArraySize = 1;
		result.MipLevels = 1;
		result.Format = DXGI_FORMAT_UNKNOWN;
		result.SampleDesc.Count = 1;
		result.SampleDesc.Quality = 0;
		result.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		result.Flags = D3D12_RESOURCE_FLAG_NONE; //TODO (?)
	}
	else if (source.type == gxapi::eResourceType::TEXTURE) {
		const auto& tex = source.textureDesc;

		result.Dimension = native_cast(tex.dimension);
		result.Width = tex.width;
		result.Height = tex.height;
		result.DepthOrArraySize = tex.depthOrArraySize;
		result.MipLevels = tex.mipLevels;
		result.Format = native_cast(tex.format);
		result.SampleDesc.Count = tex.multisampleCount;
		result.SampleDesc.Quality = tex.multisampleQuality;
		result.Layout = native_cast(tex.layout);
		result.Flags = native_cast(tex.flags);
	}

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


//---------------
//OTHER

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
	switch (source) {
	case DXGI_FORMAT_UNKNOWN:
		return gxapi::eFormat::UNKNOWN;
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		return gxapi::eFormat::R32G32B32A32_TYPELESS;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return gxapi::eFormat::R32G32B32A32_FLOAT;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		return gxapi::eFormat::R32G32B32A32_UINT;
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return gxapi::eFormat::R32G32B32A32_SINT;
	case DXGI_FORMAT_R32G32B32_TYPELESS:
		return gxapi::eFormat::R32G32B32_TYPELESS;
	case DXGI_FORMAT_R32G32B32_FLOAT:
		return gxapi::eFormat::R32G32B32_FLOAT;
	case DXGI_FORMAT_R32G32B32_UINT:
		return gxapi::eFormat::R32G32B32_UINT;
	case DXGI_FORMAT_R32G32B32_SINT:
		return gxapi::eFormat::R32G32B32_SINT;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		return gxapi::eFormat::R16G16B16A16_TYPELESS;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return gxapi::eFormat::R16G16B16A16_FLOAT;
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return gxapi::eFormat::R16G16B16A16_UNORM;
	case DXGI_FORMAT_R16G16B16A16_UINT:
		return gxapi::eFormat::R16G16B16A16_UINT;
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		return gxapi::eFormat::R16G16B16A16_SNORM;
	case DXGI_FORMAT_R16G16B16A16_SINT:
		return gxapi::eFormat::R16G16B16A16_SINT;
	case DXGI_FORMAT_R32G32_TYPELESS:
		return gxapi::eFormat::R32G32_TYPELESS;
	case DXGI_FORMAT_R32G32_FLOAT:
		return gxapi::eFormat::R32G32_FLOAT;
	case DXGI_FORMAT_R32G32_UINT:
		return gxapi::eFormat::R32G32_UINT;
	case DXGI_FORMAT_R32G32_SINT:
		return gxapi::eFormat::R32G32_SINT;
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		return gxapi::eFormat::R32G8X24_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return gxapi::eFormat::D32_FLOAT_S8X24_UINT;
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		return gxapi::eFormat::R32_FLOAT_X8X24_TYPELESS;
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return gxapi::eFormat::X32_TYPELESS_G8X24_UINT;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		return gxapi::eFormat::R10G10B10A2_TYPELESS;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return gxapi::eFormat::R10G10B10A2_UNORM;
	case DXGI_FORMAT_R10G10B10A2_UINT:
		return gxapi::eFormat::R10G10B10A2_UINT;
	case DXGI_FORMAT_R11G11B10_FLOAT:
		return gxapi::eFormat::R11G11B10_FLOAT;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return gxapi::eFormat::R8G8B8A8_TYPELESS;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return gxapi::eFormat::R8G8B8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return gxapi::eFormat::R8G8B8A8_UNORM_SRGB;
	case DXGI_FORMAT_R8G8B8A8_UINT:
		return gxapi::eFormat::R8G8B8A8_UINT;
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return gxapi::eFormat::R8G8B8A8_SNORM;
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return gxapi::eFormat::R8G8B8A8_SINT;
	case DXGI_FORMAT_R16G16_TYPELESS:
		return gxapi::eFormat::R16G16_TYPELESS;
	case DXGI_FORMAT_R16G16_FLOAT:
		return gxapi::eFormat::R16G16_FLOAT;
	case DXGI_FORMAT_R16G16_UNORM:
		return gxapi::eFormat::R16G16_UNORM;
	case DXGI_FORMAT_R16G16_UINT:
		return gxapi::eFormat::R16G16_UINT;
	case DXGI_FORMAT_R16G16_SNORM:
		return gxapi::eFormat::R16G16_SNORM;
	case DXGI_FORMAT_R16G16_SINT:
		return gxapi::eFormat::R16G16_SINT;
	case DXGI_FORMAT_R32_TYPELESS:
		return gxapi::eFormat::R32_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT:
		return gxapi::eFormat::D32_FLOAT;
	case DXGI_FORMAT_R32_FLOAT:
		return gxapi::eFormat::R32_FLOAT;
	case DXGI_FORMAT_R32_UINT:
		return gxapi::eFormat::R32_UINT;
	case DXGI_FORMAT_R32_SINT:
		return gxapi::eFormat::R32_SINT;
	case DXGI_FORMAT_R24G8_TYPELESS:
		return gxapi::eFormat::R24G8_TYPELESS;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return gxapi::eFormat::D24_UNORM_S8_UINT;
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		return gxapi::eFormat::R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return gxapi::eFormat::X24_TYPELESS_G8_UINT;
	case DXGI_FORMAT_R8G8_TYPELESS:
		return gxapi::eFormat::R8G8_TYPELESS;
	case DXGI_FORMAT_R8G8_UNORM:
		return gxapi::eFormat::R8G8_UNORM;
	case DXGI_FORMAT_R8G8_UINT:
		return gxapi::eFormat::R8G8_UINT;
	case DXGI_FORMAT_R8G8_SNORM:
		return gxapi::eFormat::R8G8_SNORM;
	case DXGI_FORMAT_R8G8_SINT:
		return gxapi::eFormat::R8G8_SINT;
	case DXGI_FORMAT_R16_TYPELESS:
		return gxapi::eFormat::R16_TYPELESS;
	case DXGI_FORMAT_R16_FLOAT:
		return gxapi::eFormat::R16_FLOAT;
	case DXGI_FORMAT_D16_UNORM:
		return gxapi::eFormat::D16_UNORM;
	case DXGI_FORMAT_R16_UNORM:
		return gxapi::eFormat::R16_UNORM;
	case DXGI_FORMAT_R16_UINT:
		return gxapi::eFormat::R16_UINT;
	case DXGI_FORMAT_R16_SNORM:
		return gxapi::eFormat::R16_SNORM;
	case DXGI_FORMAT_R16_SINT:
		return gxapi::eFormat::R16_SINT;
	case DXGI_FORMAT_R8_TYPELESS:
		return gxapi::eFormat::R8_TYPELESS;
	case DXGI_FORMAT_R8_UNORM:
		return gxapi::eFormat::R8_UNORM;
	case DXGI_FORMAT_R8_UINT:
		return gxapi::eFormat::R8_UINT;
	case DXGI_FORMAT_R8_SNORM:
		return gxapi::eFormat::R8_SNORM;
	case DXGI_FORMAT_R8_SINT:
		return gxapi::eFormat::R8_SINT;
	case DXGI_FORMAT_A8_UNORM:
		return gxapi::eFormat::A8_UNORM;
	default:
		assert(false);
		break;
	}

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
		result.bufferDesc.sizeInBytes = source.Width;
	}
	else if (result.type == gxapi::eResourceType::TEXTURE) {
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
