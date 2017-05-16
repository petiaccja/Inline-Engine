#include "NativeCast.hpp"

#include "../GraphicsApi_LL/Common.hpp"

#include <cassert>
#include <d3dcompiler.h>

//Dont even think about returning a pointer that points to a locally allocated space
//#include <vector>

namespace inl {
namespace gxapi_dx12 {


///////////////////////
// SPECIAL
void* native_cast_ptr(std::uintptr_t source) {
	return reinterpret_cast<void*>(source);
}

std::uintptr_t native_cast_ptr(const void* source) {
	return reinterpret_cast<std::uintptr_t>(source);
}


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


const ID3D12Resource* native_cast(const gxapi::IResource* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<const Resource*>(source)->GetNative();
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

	return dynamic_cast<GraphicsCommandList*>(source)->GetNative();
}


ID3D12GraphicsCommandList* native_cast(gxapi::IComputeCommandList* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return dynamic_cast<ComputeCommandList*>(source)->GetNative();
}


ID3D12GraphicsCommandList* native_cast(gxapi::ICopyCommandList* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return dynamic_cast<CopyCommandList*>(source)->GetNative();
}


ID3D12RootSignature* native_cast(gxapi::IRootSignature* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<RootSignature*>(source)->GetNative();
}


ID3D12DescriptorHeap* native_cast(gxapi::IDescriptorHeap* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<DescriptorHeap*>(source)->GetNative();
}


ID3D12Fence* native_cast(gxapi::IFence * source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<Fence*>(source)->GetNative();
}

ID3D12CommandQueue* native_cast(gxapi::ICommandQueue* source) {
	if (source == nullptr) {
		return nullptr;
	}

	return static_cast<CommandQueue*>(source)->GetNative();
}


//---------------
//ENUM

D3D12_SHADER_VISIBILITY native_cast(gxapi::eShaderVisiblity source) {
	switch (source) {
	case gxapi::eShaderVisiblity::ALL:
		return D3D12_SHADER_VISIBILITY_ALL;
	case gxapi::eShaderVisiblity::VERTEX:
		return D3D12_SHADER_VISIBILITY_VERTEX;
	case gxapi::eShaderVisiblity::GEOMETRY:
		return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case gxapi::eShaderVisiblity::HULL:
		return D3D12_SHADER_VISIBILITY_HULL;
	case gxapi::eShaderVisiblity::DOMAIN:
		return D3D12_SHADER_VISIBILITY_DOMAIN;
	case gxapi::eShaderVisiblity::PIXEL:
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
		break;
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
		break;
	}

	return D3D12_COMMAND_LIST_TYPE{};
}


D3D12_DESCRIPTOR_HEAP_TYPE native_cast(gxapi::eDescriptorHeapType source) {
	using gxapi::eDescriptorHeapType;
	switch (source) {
	case eDescriptorHeapType::CBV_SRV_UAV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	case eDescriptorHeapType::SAMPLER:
		return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	case eDescriptorHeapType::RTV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	case eDescriptorHeapType::DSV:
		return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	default:
		assert(false);
		break;
	}

	return D3D12_DESCRIPTOR_HEAP_TYPE{};
}


D3D12_ROOT_PARAMETER_TYPE native_cast(gxapi::RootParameterDesc::eType source) {
	switch (source) {
	case gxapi::RootParameterDesc::CONSTANT:
		return D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	case gxapi::RootParameterDesc::CBV:
		return D3D12_ROOT_PARAMETER_TYPE_CBV;
	case gxapi::RootParameterDesc::SRV:
		return D3D12_ROOT_PARAMETER_TYPE_SRV;
	case gxapi::RootParameterDesc::UAV:
		return D3D12_ROOT_PARAMETER_TYPE_UAV;
	case gxapi::RootParameterDesc::DESCRIPTOR_TABLE:
		return D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	default:
		assert(false);
		break;
	}

	return D3D12_ROOT_PARAMETER_TYPE{};
}

D3D12_DESCRIPTOR_RANGE_TYPE native_cast(gxapi::DescriptorRange::eType source) {
	switch (source) {
	case gxapi::DescriptorRange::CBV:
		return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	case gxapi::DescriptorRange::SRV:
		return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	case gxapi::DescriptorRange::UAV:
		return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	case gxapi::DescriptorRange::SAMPLER:
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
	case gxapi::eComparisonFunction::NEVER:
		return D3D12_COMPARISON_FUNC_NEVER;
	case gxapi::eComparisonFunction::LESS:
		return D3D12_COMPARISON_FUNC_LESS;
	case gxapi::eComparisonFunction::LESS_EQUAL:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case gxapi::eComparisonFunction::GREATER:
		return D3D12_COMPARISON_FUNC_GREATER;
	case gxapi::eComparisonFunction::GREATER_EQUAL:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case gxapi::eComparisonFunction::EQUAL:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case gxapi::eComparisonFunction::NOT_EQUAL:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case gxapi::eComparisonFunction::ALWAYS:
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
	case gxapi::eHeapType::DEFAULT:
		return D3D12_HEAP_TYPE_DEFAULT;
	case gxapi::eHeapType::UPLOAD:
		return D3D12_HEAP_TYPE_UPLOAD;
	case gxapi::eHeapType::READBACK:
		return D3D12_HEAP_TYPE_READBACK;
	case gxapi::eHeapType::CUSTOM:
		return D3D12_HEAP_TYPE_CUSTOM;
	default:
		assert(false);
		break;
	}

	return D3D12_HEAP_TYPE{};
}


D3D12_CPU_PAGE_PROPERTY native_cast(gxapi::eCpuPageProperty source) {
	switch (source) {
	case gxapi::eCpuPageProperty::UNKNOWN:
		return D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	case gxapi::eCpuPageProperty::NOT_AVAILABLE:
		return D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
	case gxapi::eCpuPageProperty::WRITE_COMBINE:
		return D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
	case gxapi::eCpuPageProperty::WRITE_BACK:
		return D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	default:
		assert(false);
		break;
	}

	return D3D12_CPU_PAGE_PROPERTY{};
}


D3D12_MEMORY_POOL native_cast(gxapi::eMemoryPool source) {
	switch (source) {
	case gxapi::eMemoryPool::UNKNOWN:
		return D3D12_MEMORY_POOL_UNKNOWN;
	case gxapi::eMemoryPool::HOST:
		return D3D12_MEMORY_POOL_L0;
	case gxapi::eMemoryPool::DEDICATED:
		return D3D12_MEMORY_POOL_L1;
	default:
		assert(false);
		break;
	}

	return D3D12_MEMORY_POOL{};
}


D3D12_STATIC_BORDER_COLOR native_cast(gxapi::eTextureBorderColor source) {
	switch (source) {
	case gxapi::eTextureBorderColor::TRANSPARENT_BLACK:
		return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	case gxapi::eTextureBorderColor::OPAQUE_BLACK:
		return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	case gxapi::eTextureBorderColor::OPAQUE_WHITE:
		return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	default:
		assert(false);
		break;
	}

	return D3D12_STATIC_BORDER_COLOR{};
}


DXGI_FORMAT native_cast(gxapi::eFormat source) {
	switch (source) {
	case gxapi::eFormat::UNKNOWN:
		return DXGI_FORMAT_UNKNOWN;
	case gxapi::eFormat::R32G32B32A32_TYPELESS:
		return DXGI_FORMAT_R32G32B32A32_TYPELESS;
	case gxapi::eFormat::R32G32B32A32_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case gxapi::eFormat::R32G32B32A32_UINT:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case gxapi::eFormat::R32G32B32A32_SINT:
		return DXGI_FORMAT_R32G32B32A32_SINT;
	case gxapi::eFormat::R32G32B32_TYPELESS:
		return DXGI_FORMAT_R32G32B32_TYPELESS;
	case gxapi::eFormat::R32G32B32_FLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case gxapi::eFormat::R32G32B32_UINT:
		return DXGI_FORMAT_R32G32B32_UINT;
	case gxapi::eFormat::R32G32B32_SINT:
		return DXGI_FORMAT_R32G32B32_SINT;
	case gxapi::eFormat::R16G16B16A16_TYPELESS:
		return DXGI_FORMAT_R16G16B16A16_TYPELESS;
	case gxapi::eFormat::R16G16B16A16_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case gxapi::eFormat::R16G16B16A16_UNORM:
		return DXGI_FORMAT_R16G16B16A16_UNORM;
	case gxapi::eFormat::R16G16B16A16_UINT:
		return DXGI_FORMAT_R16G16B16A16_UINT;
	case gxapi::eFormat::R16G16B16A16_SNORM:
		return DXGI_FORMAT_R16G16B16A16_SNORM;
	case gxapi::eFormat::R16G16B16A16_SINT:
		return DXGI_FORMAT_R16G16B16A16_SINT;
	case gxapi::eFormat::R32G32_TYPELESS:
		return DXGI_FORMAT_R32G32_TYPELESS;
	case gxapi::eFormat::R32G32_FLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
	case gxapi::eFormat::R32G32_UINT:
		return DXGI_FORMAT_R32G32_UINT;
	case gxapi::eFormat::R32G32_SINT:
		return DXGI_FORMAT_R32G32_SINT;
	case gxapi::eFormat::R32G8X24_TYPELESS:
		return DXGI_FORMAT_R32G8X24_TYPELESS;
	case gxapi::eFormat::D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case gxapi::eFormat::R32_FLOAT_X8X24_TYPELESS:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	case gxapi::eFormat::X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
	case gxapi::eFormat::R10G10B10A2_TYPELESS:
		return DXGI_FORMAT_R10G10B10A2_TYPELESS;
	case gxapi::eFormat::R10G10B10A2_UNORM:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case gxapi::eFormat::R10G10B10A2_UINT:
		return DXGI_FORMAT_R10G10B10A2_UINT;
	case gxapi::eFormat::R11G11B10_FLOAT:
		return DXGI_FORMAT_R11G11B10_FLOAT;
	case gxapi::eFormat::R8G8B8A8_TYPELESS:
		return DXGI_FORMAT_R8G8B8A8_TYPELESS;
	case gxapi::eFormat::R8G8B8A8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case gxapi::eFormat::R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case gxapi::eFormat::R8G8B8A8_UINT:
		return DXGI_FORMAT_R8G8B8A8_UINT;
	case gxapi::eFormat::R8G8B8A8_SNORM:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
	case gxapi::eFormat::R8G8B8A8_SINT:
		return DXGI_FORMAT_R8G8B8A8_SINT;
	case gxapi::eFormat::R16G16_TYPELESS:
		return DXGI_FORMAT_R16G16_TYPELESS;
	case gxapi::eFormat::R16G16_FLOAT:
		return DXGI_FORMAT_R16G16_FLOAT;
	case gxapi::eFormat::R16G16_UNORM:
		return DXGI_FORMAT_R16G16_UNORM;
	case gxapi::eFormat::R16G16_UINT:
		return DXGI_FORMAT_R16G16_UINT;
	case gxapi::eFormat::R16G16_SNORM:
		return DXGI_FORMAT_R16G16_SNORM;
	case gxapi::eFormat::R16G16_SINT:
		return DXGI_FORMAT_R16G16_SINT;
	case gxapi::eFormat::R32_TYPELESS:
		return DXGI_FORMAT_R32_TYPELESS;
	case gxapi::eFormat::D32_FLOAT:
		return DXGI_FORMAT_D32_FLOAT;
	case gxapi::eFormat::R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case gxapi::eFormat::R32_UINT:
		return DXGI_FORMAT_R32_UINT;
	case gxapi::eFormat::R32_SINT:
		return DXGI_FORMAT_R32_SINT;
	case gxapi::eFormat::R24G8_TYPELESS:
		return DXGI_FORMAT_R24G8_TYPELESS;
	case gxapi::eFormat::D24_UNORM_S8_UINT:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case gxapi::eFormat::R24_UNORM_X8_TYPELESS:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case gxapi::eFormat::X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
	case gxapi::eFormat::R8G8_TYPELESS:
		return DXGI_FORMAT_R8G8_TYPELESS;
	case gxapi::eFormat::R8G8_UNORM:
		return DXGI_FORMAT_R8G8_UNORM;
	case gxapi::eFormat::R8G8_UINT:
		return DXGI_FORMAT_R8G8_UINT;
	case gxapi::eFormat::R8G8_SNORM:
		return DXGI_FORMAT_R8G8_SNORM;
	case gxapi::eFormat::R8G8_SINT:
		return DXGI_FORMAT_R8G8_SINT;
	case gxapi::eFormat::R16_TYPELESS:
		return DXGI_FORMAT_R16_TYPELESS;
	case gxapi::eFormat::R16_FLOAT:
		return DXGI_FORMAT_R16_FLOAT;
	case gxapi::eFormat::D16_UNORM:
		return DXGI_FORMAT_D16_UNORM;
	case gxapi::eFormat::R16_UNORM:
		return DXGI_FORMAT_R16_UNORM;
	case gxapi::eFormat::R16_UINT:
		return DXGI_FORMAT_R16_UINT;
	case gxapi::eFormat::R16_SNORM:
		return DXGI_FORMAT_R16_SNORM;
	case gxapi::eFormat::R16_SINT:
		return DXGI_FORMAT_R16_SINT;
	case gxapi::eFormat::R8_TYPELESS:
		return DXGI_FORMAT_R8_TYPELESS;
	case gxapi::eFormat::R8_UNORM:
		return DXGI_FORMAT_R8_UNORM;
	case gxapi::eFormat::R8_UINT:
		return DXGI_FORMAT_R8_UINT;
	case gxapi::eFormat::R8_SNORM:
		return DXGI_FORMAT_R8_SNORM;
	case gxapi::eFormat::R8_SINT:
		return DXGI_FORMAT_R8_SINT;
	case gxapi::eFormat::A8_UNORM:
		return DXGI_FORMAT_A8_UNORM;

	default:
		assert(false);
		break;
	}

	return DXGI_FORMAT{};
}


D3D12_TEXTURE_LAYOUT native_cast(gxapi::eTextureLayout source) {
	switch (source) {
	case gxapi::eTextureLayout::UNKNOWN:
		return D3D12_TEXTURE_LAYOUT_UNKNOWN;
	case gxapi::eTextureLayout::ROW_MAJOR:
		return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	case gxapi::eTextureLayout::UNDEFINED_SWIZZLE:
		return D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
	case gxapi::eTextureLayout::STANDARD_SWIZZLE:
		return D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE;
	default:
		assert(false);
		break;
	}

	return D3D12_TEXTURE_LAYOUT{};
}


D3D12_RESOURCE_DIMENSION native_cast(gxapi::eTextueDimension source) {
	switch (source) {
	case gxapi::eTextueDimension::ONE:
		return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	case gxapi::eTextueDimension::TWO:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case gxapi::eTextueDimension::THREE:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	default:
		assert(false);
		break;
	}

	return D3D12_RESOURCE_DIMENSION{};
}


D3D12_BLEND native_cast(gxapi::eBlendOperand source) {
	switch (source) {
	case gxapi::eBlendOperand::ZERO:
		return D3D12_BLEND_ZERO;
	case gxapi::eBlendOperand::ONE:
		return D3D12_BLEND_ONE;
	case gxapi::eBlendOperand::SHADER_OUT:
		return D3D12_BLEND_SRC_COLOR;
	case gxapi::eBlendOperand::INV_SHADER_OUT:
		return D3D12_BLEND_INV_SRC_COLOR;
	case gxapi::eBlendOperand::SHADER_ALPHA:
		return D3D12_BLEND_SRC_ALPHA;
	case gxapi::eBlendOperand::INV_SHADER_ALPHA:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case gxapi::eBlendOperand::TARGET_OUT:
		return D3D12_BLEND_DEST_COLOR;
	case gxapi::eBlendOperand::INV_TARGET_OUT:
		return D3D12_BLEND_INV_DEST_COLOR;
	case gxapi::eBlendOperand::TARGET_ALPHA:
		return D3D12_BLEND_DEST_ALPHA;
	case gxapi::eBlendOperand::INV_TARGET_ALPHA:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case gxapi::eBlendOperand::SHADER_ALPHA_SAT:
		return D3D12_BLEND_SRC_ALPHA_SAT;
	case gxapi::eBlendOperand::BLEND_FACTOR:
		return D3D12_BLEND_BLEND_FACTOR;
	case gxapi::eBlendOperand::INV_BLEND_FACTOR:
		return D3D12_BLEND_INV_BLEND_FACTOR;
	default:
		assert(false);
		break;
	}

	return D3D12_BLEND{};
}


D3D12_BLEND_OP native_cast(gxapi::eBlendOperation source) {
	switch (source) {
	case gxapi::eBlendOperation::ADD:
		return D3D12_BLEND_OP_ADD;
	case gxapi::eBlendOperation::SUBTRACT:
		return D3D12_BLEND_OP_SUBTRACT;
	case gxapi::eBlendOperation::REVERSE_SUBTRACT:
		return D3D12_BLEND_OP_REV_SUBTRACT;
	case gxapi::eBlendOperation::MIN:
		return D3D12_BLEND_OP_MIN;
	case gxapi::eBlendOperation::MAX:
		return D3D12_BLEND_OP_MAX;
	default:
		assert(false);
		break;
	}

	return D3D12_BLEND_OP{};
}


D3D12_LOGIC_OP native_cast(gxapi::eBlendLogicOperation source) {
	switch (source) {
	case gxapi::eBlendLogicOperation::CLEAR:
		return D3D12_LOGIC_OP_CLEAR;
	case gxapi::eBlendLogicOperation::SET:
		return D3D12_LOGIC_OP_SET;
	case gxapi::eBlendLogicOperation::COPY:
		return D3D12_LOGIC_OP_COPY;
	case gxapi::eBlendLogicOperation::COPY_INVERTED:
		return D3D12_LOGIC_OP_COPY_INVERTED;
	case gxapi::eBlendLogicOperation::NOOP:
		return D3D12_LOGIC_OP_NOOP;
	case gxapi::eBlendLogicOperation::INVERT:
		return D3D12_LOGIC_OP_INVERT;
	case gxapi::eBlendLogicOperation::AND:
		return D3D12_LOGIC_OP_AND;
	case gxapi::eBlendLogicOperation::NAND:
		return D3D12_LOGIC_OP_NAND;
	case gxapi::eBlendLogicOperation::OR:
		return D3D12_LOGIC_OP_OR;
	case gxapi::eBlendLogicOperation::NOR:
		return D3D12_LOGIC_OP_NOR;
	case gxapi::eBlendLogicOperation::XOR:
		return D3D12_LOGIC_OP_XOR;
	case gxapi::eBlendLogicOperation::EQUIV:
		return D3D12_LOGIC_OP_EQUIV;
	case gxapi::eBlendLogicOperation::AND_REVERSE:
		return D3D12_LOGIC_OP_AND_REVERSE;
	case gxapi::eBlendLogicOperation::AND_INVERTED:
		return D3D12_LOGIC_OP_AND_INVERTED;
	case gxapi::eBlendLogicOperation::OR_REVERSE:
		return D3D12_LOGIC_OP_OR_REVERSE;
	case gxapi::eBlendLogicOperation::OR_INVERTED:
		return D3D12_LOGIC_OP_OR_INVERTED;
	default:
		assert(false);
		break;
	}

	return D3D12_LOGIC_OP{};
}


D3D12_FILL_MODE native_cast(gxapi::eFillMode source) {
	switch (source) {
	case gxapi::eFillMode::WIREFRAME:
		return D3D12_FILL_MODE_WIREFRAME;
	case gxapi::eFillMode::SOLID:
		return D3D12_FILL_MODE_SOLID;
	default:
		assert(false);
		break;
	}

	return D3D12_FILL_MODE{};
}


D3D12_CULL_MODE native_cast(gxapi::eCullMode source) {
	static_assert(frontFaceIsCCW == true, "Front face expected to be counter clockwise");

	switch (source) {
	case gxapi::eCullMode::DRAW_ALL:
		return D3D12_CULL_MODE_NONE;
	case gxapi::eCullMode::DRAW_CW:
		return D3D12_CULL_MODE_FRONT;
	case gxapi::eCullMode::DRAW_CCW:
		return D3D12_CULL_MODE_BACK;
	default:
		assert(false);
		break;
	}

	return D3D12_CULL_MODE{};
}


D3D12_CONSERVATIVE_RASTERIZATION_MODE native_cast(gxapi::eConservativeRasterizationMode source) {
	switch (source) {
	case gxapi::eConservativeRasterizationMode::ON:
		return D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
	case gxapi::eConservativeRasterizationMode::OFF:
		return D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	default:
		assert(false);
		break;
	}

	return D3D12_CONSERVATIVE_RASTERIZATION_MODE{};
}


D3D12_STENCIL_OP native_cast(gxapi::eStencilOp source) {
	switch (source) {
	case gxapi::eStencilOp::KEEP:
		return D3D12_STENCIL_OP_KEEP;
	case gxapi::eStencilOp::ZERO:
		return D3D12_STENCIL_OP_ZERO;
	case gxapi::eStencilOp::REPLACE:
		return D3D12_STENCIL_OP_REPLACE;
	case gxapi::eStencilOp::INCR_SAT:
		return D3D12_STENCIL_OP_INCR_SAT;
	case gxapi::eStencilOp::DECR_SAT:
		return D3D12_STENCIL_OP_DECR_SAT;
	case gxapi::eStencilOp::INCR_WRAP:
		return D3D12_STENCIL_OP_INCR;
	case gxapi::eStencilOp::DECR_WRAP:
		return D3D12_STENCIL_OP_DECR;
	case gxapi::eStencilOp::INVERT:
		return D3D12_STENCIL_OP_INVERT;
	default:
		assert(false);
		break;
	}

	return D3D12_STENCIL_OP{};
}


D3D12_INDEX_BUFFER_STRIP_CUT_VALUE native_cast(gxapi::eTriangleStripCutIndex source) {
	switch (source) {
	case gxapi::eTriangleStripCutIndex::DISABLED:
		return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	case gxapi::eTriangleStripCutIndex::FFFFh:
		return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
	case gxapi::eTriangleStripCutIndex::FFFFFFFFh:
		return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
	default:
		assert(false);
		break;
	}

	return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE{};
}


D3D12_PRIMITIVE_TOPOLOGY_TYPE native_cast(gxapi::ePrimitiveTopologyType source) {
	switch (source) {
	case gxapi::ePrimitiveTopologyType::UNDEFINED:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	case gxapi::ePrimitiveTopologyType::POINT:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case gxapi::ePrimitiveTopologyType::LINE:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case gxapi::ePrimitiveTopologyType::TRIANGLE:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case gxapi::ePrimitiveTopologyType::PATCH:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	default:
		assert(false);
		break;
	}

	return D3D12_PRIMITIVE_TOPOLOGY_TYPE{};
}


D3D12_INPUT_CLASSIFICATION native_cast(gxapi::eInputClassification source) {
	switch (source) {
	case gxapi::eInputClassification::VERTEX_DATA:
		return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	case gxapi::eInputClassification::INSTANCE_DATA:
		return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
	default:
		assert(false);
		break;
	}

	return D3D12_INPUT_CLASSIFICATION{};
}


D3D12_DSV_DIMENSION native_cast(gxapi::eDsvDimension source) {
	switch (source) {
	case gxapi::eDsvDimension::UNKNOWN:
		return D3D12_DSV_DIMENSION_UNKNOWN;
	case gxapi::eDsvDimension::TEXTURE1D:
		return D3D12_DSV_DIMENSION_TEXTURE1D;
	case gxapi::eDsvDimension::TEXTURE1DARRAY:
		return D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
	case gxapi::eDsvDimension::TEXTURE2D:
		return D3D12_DSV_DIMENSION_TEXTURE2D;
	case gxapi::eDsvDimension::TEXTURE2DARRAY:
		return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	case gxapi::eDsvDimension::TEXTURE2DMS:
		return D3D12_DSV_DIMENSION_TEXTURE2DMS;
	case gxapi::eDsvDimension::TEXTURE2DMSARRAY:
		return D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
	default:
		assert(false);
		break;
	}

	return D3D12_DSV_DIMENSION{};
}


D3D12_RTV_DIMENSION native_cast(gxapi::eRtvDimension source) {
	switch (source) {
	case gxapi::eRtvDimension::UNKNOWN:
		return D3D12_RTV_DIMENSION_UNKNOWN;
	case gxapi::eRtvDimension::BUFFER:
		return D3D12_RTV_DIMENSION_BUFFER;
	case gxapi::eRtvDimension::TEXTURE1D:
		return D3D12_RTV_DIMENSION_TEXTURE1D;
	case gxapi::eRtvDimension::TEXTURE1DARRAY:
		return D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
	case gxapi::eRtvDimension::TEXTURE2D:
		return D3D12_RTV_DIMENSION_TEXTURE2D;
	case gxapi::eRtvDimension::TEXTURE2DARRAY:
		return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	case gxapi::eRtvDimension::TEXTURE2DMS:
		return D3D12_RTV_DIMENSION_TEXTURE2DMS;
	case gxapi::eRtvDimension::TEXTURE2DMSARRAY:
		return D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
	case gxapi::eRtvDimension::TEXTURE3D:
		return D3D12_RTV_DIMENSION_TEXTURE3D;
		break;
	default:
		assert(false);
		break;
	}

	return D3D12_RTV_DIMENSION{};
}


D3D12_SRV_DIMENSION native_cast(gxapi::eSrvDimension source) {
	switch (source) {
	case gxapi::eSrvDimension::UNKNOWN:
		assert(false);
		break;
	case gxapi::eSrvDimension::BUFFER:
		return D3D12_SRV_DIMENSION_BUFFER;
	case gxapi::eSrvDimension::TEXTURE1D:
		return D3D12_SRV_DIMENSION_TEXTURE1D;
	case gxapi::eSrvDimension::TEXTURE1DARRAY:
		return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
	case gxapi::eSrvDimension::TEXTURE2D:
		return D3D12_SRV_DIMENSION_TEXTURE2D;
	case gxapi::eSrvDimension::TEXTURE2DARRAY:
		return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	case gxapi::eSrvDimension::TEXTURE2DMS:
		return D3D12_SRV_DIMENSION_TEXTURE2DMS;
	case gxapi::eSrvDimension::TEXTURE2DMSARRAY:
		return D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
	case gxapi::eSrvDimension::TEXTURE3D:
		return D3D12_SRV_DIMENSION_TEXTURE3D;
	case gxapi::eSrvDimension::TEXTURECUBE:
		return D3D12_SRV_DIMENSION_TEXTURECUBE;
	case gxapi::eSrvDimension::TEXTURECUBEARRAY:
		return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	default:
		assert(false);
		break;
	}

	return D3D12_SRV_DIMENSION{};
}


D3D12_UAV_DIMENSION native_cast(gxapi::eUavDimension source) {
	switch (source) {
		case gxapi::eUavDimension::UNKNOWN:
			assert(false);
			break;
		case gxapi::eUavDimension::BUFFER:
			return D3D12_UAV_DIMENSION_BUFFER;
		case gxapi::eUavDimension::TEXTURE1D:
			return D3D12_UAV_DIMENSION_TEXTURE1D;
		case gxapi::eUavDimension::TEXTURE1DARRAY:
			return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
		case gxapi::eUavDimension::TEXTURE2D:
			return D3D12_UAV_DIMENSION_TEXTURE2D;
		case gxapi::eUavDimension::TEXTURE2DARRAY:
			return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		case gxapi::eUavDimension::TEXTURE3D:
			return D3D12_UAV_DIMENSION_TEXTURE3D;
		default:
			assert(false);
			break;
	}

	return D3D12_UAV_DIMENSION{};
}


D3D12_RESOURCE_BARRIER_FLAGS native_cast(gxapi::eResourceBarrierSplit source) {
	switch (source) {
		case gxapi::eResourceBarrierSplit::BEGIN:
			return D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		case gxapi::eResourceBarrierSplit::END:
			return D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
		case gxapi::eResourceBarrierSplit::NORMAL:
			return D3D12_RESOURCE_BARRIER_FLAG_NONE;
		default:
			return D3D12_RESOURCE_BARRIER_FLAGS(0);
	}
}

D3D12_RESOURCE_BARRIER_TYPE native_cast(gxapi::eResourceBarrierType source) {
	switch (source)
	{
		case inl::gxapi::eResourceBarrierType::TRANSITION:
			return D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		case inl::gxapi::eResourceBarrierType::ALIASING:
			return D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		case inl::gxapi::eResourceBarrierType::UAV:
			return D3D12_RESOURCE_BARRIER_TYPE_UAV;
		default:
			return D3D12_RESOURCE_BARRIER_TYPE(0);
	}
}


//---------------
//FLAGS

D3D12_RESOURCE_FLAGS native_cast(gxapi::eResourceFlags source) {
	D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;

	result |= D3D12_RESOURCE_FLAGS(bool(source & gxapi::eResourceFlags::ALLOW_RENDER_TARGET) * D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	result |= D3D12_RESOURCE_FLAGS(bool(source & gxapi::eResourceFlags::ALLOW_DEPTH_STENCIL) * D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	result |= D3D12_RESOURCE_FLAGS(bool(source & gxapi::eResourceFlags::ALLOW_UNORDERED_ACCESS) * D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	result |= D3D12_RESOURCE_FLAGS(bool(source & gxapi::eResourceFlags::DENY_SHADER_RESOURCE) * D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
	result |= D3D12_RESOURCE_FLAGS(bool(source & gxapi::eResourceFlags::ALLOW_CROSS_ADAPTER) * D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER);
	result |= D3D12_RESOURCE_FLAGS(bool(source & gxapi::eResourceFlags::ALLOW_SIMULTANEOUS_ACCESS) * D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS);

	return result;
}


D3D12_HEAP_FLAGS native_cast(gxapi::eHeapFlags source) {
	D3D12_HEAP_FLAGS result = D3D12_HEAP_FLAG_NONE;

	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::SHARED) * D3D12_HEAP_FLAG_SHARED);
	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::DENY_BUFFERS) * D3D12_HEAP_FLAG_DENY_BUFFERS);
	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::ALLOW_DISPLAY) * D3D12_HEAP_FLAG_ALLOW_DISPLAY);
	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::SHARED_CROSS_ADAPTER) * D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER);
	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::DENY_RT_DS_TEXTURES) * D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES);
	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::DENY_NON_RT_DS_TEXTURES) * D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES);
	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::ALLOW_ALL_BUFFERS_AND_TEXTURES) * D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES);
	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::ALLOW_ONLY_BUFFERS) * D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);
	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::ALLOW_ONLY_NON_RT_DS_TEXTURES) * D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);
	result |= D3D12_HEAP_FLAGS(bool(source & gxapi::eHeapFlags::ALLOW_ONLY_RT_DS_TEXTURES) * D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);

	return result;
}


D3D12_RESOURCE_STATES native_cast(gxapi::eResourceState source) {
	D3D12_RESOURCE_STATES nativeFlag = static_cast<D3D12_RESOURCE_STATES>(0);

	// native  |= (boolean cast -> 1 if true, 0 if false ) * native value;
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::COMMON) * D3D12_RESOURCE_STATE_COMMON);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER) * D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::INDEX_BUFFER) * D3D12_RESOURCE_STATE_INDEX_BUFFER);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::RENDER_TARGET) * D3D12_RESOURCE_STATE_RENDER_TARGET);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::UNORDERED_ACCESS) * D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::DEPTH_WRITE) * D3D12_RESOURCE_STATE_DEPTH_WRITE);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::DEPTH_READ) * D3D12_RESOURCE_STATE_DEPTH_READ);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE) * D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::PIXEL_SHADER_RESOURCE) * D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::STREAM_OUT) * D3D12_RESOURCE_STATE_STREAM_OUT);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::INDIRECT_ARGUMENT) * D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::COPY_DEST) * D3D12_RESOURCE_STATE_COPY_DEST);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::COPY_SOURCE) * D3D12_RESOURCE_STATE_COPY_SOURCE);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::RESOLVE_DEST) * D3D12_RESOURCE_STATE_RESOLVE_DEST);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::RESOLVE_SOURCE) * D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::GENERIC_READ) * D3D12_RESOURCE_STATE_GENERIC_READ);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::PRESENT) * D3D12_RESOURCE_STATE_PRESENT);
	nativeFlag |= D3D12_RESOURCE_STATES(bool(source & gxapi::eResourceState::PREDICATION) * D3D12_RESOURCE_STATE_PREDICATION);

	return nativeFlag;
}


UINT8 native_cast(gxapi::eColorMask source) {
	D3D12_COLOR_WRITE_ENABLE nativeFlag = static_cast<D3D12_COLOR_WRITE_ENABLE>(0);
	using UnderlyingType = typename std::underlying_type<D3D12_COLOR_WRITE_ENABLE>::type;

	// native  |= (boolean cast -> 1 if true, 0 if false ) * native value;
	(UnderlyingType&)nativeFlag |= D3D12_COLOR_WRITE_ENABLE(bool(source & gxapi::eColorMask::RED) * D3D12_COLOR_WRITE_ENABLE_RED);
	(UnderlyingType&)nativeFlag |= D3D12_COLOR_WRITE_ENABLE(bool(source & gxapi::eColorMask::GREEN) * D3D12_COLOR_WRITE_ENABLE_GREEN);
	(UnderlyingType&)nativeFlag |= D3D12_COLOR_WRITE_ENABLE(bool(source & gxapi::eColorMask::BLUE) * D3D12_COLOR_WRITE_ENABLE_BLUE);
	(UnderlyingType&)nativeFlag |= D3D12_COLOR_WRITE_ENABLE(bool(source & gxapi::eColorMask::ALPHA) * D3D12_COLOR_WRITE_ENABLE_ALPHA);
	(UnderlyingType&)nativeFlag |= D3D12_COLOR_WRITE_ENABLE(bool(source & gxapi::eColorMask::ALL) * D3D12_COLOR_WRITE_ENABLE_ALL);

	return nativeFlag;
}


D3D12_DSV_FLAGS native_cast(gxapi::eDsvFlags source) {
	D3D12_DSV_FLAGS nativeFlag = D3D12_DSV_FLAG_NONE;

	nativeFlag |= D3D12_DSV_FLAGS(bool(source & gxapi::eDsvFlags::READ_ONLY_DEPTH) * D3D12_DSV_FLAG_READ_ONLY_DEPTH);
	nativeFlag |= D3D12_DSV_FLAGS(bool(source & gxapi::eDsvFlags::READ_ONLY_STENCIL) * D3D12_DSV_FLAG_READ_ONLY_STENCIL);

	return nativeFlag;
}


UINT native_cast(gxapi::eShaderCompileFlags source) {
	UINT nativeFlag = 0;

	// native  |= (boolean cast -> 1 if true, 0 if false ) * native value;
	nativeFlag |= (bool(source & gxapi::eShaderCompileFlags::DEBUG) * D3DCOMPILE_DEBUG);
	nativeFlag |= (bool(source & gxapi::eShaderCompileFlags::NO_OPTIMIZATION) * D3DCOMPILE_OPTIMIZATION_LEVEL0);
	nativeFlag |= (bool(source & gxapi::eShaderCompileFlags::OPTIMIZATION_LOW) * D3DCOMPILE_OPTIMIZATION_LEVEL1);
	nativeFlag |= (bool(source & gxapi::eShaderCompileFlags::OPTIMIZATION_MEDIUM) * D3DCOMPILE_OPTIMIZATION_LEVEL2);
	nativeFlag |= (bool(source & gxapi::eShaderCompileFlags::OPTIMIZATION_HIGH) * D3DCOMPILE_OPTIMIZATION_LEVEL3);
	nativeFlag |= (bool(source & gxapi::eShaderCompileFlags::COLUMN_MAJOR_MATRICES) * D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR);
	nativeFlag |= (bool(source & gxapi::eShaderCompileFlags::ROW_MAJOR_MATRICES) * D3DCOMPILE_PACK_MATRIX_ROW_MAJOR);
	nativeFlag |= (bool(source & gxapi::eShaderCompileFlags::FORCE_IEEE) * D3DCOMPILE_IEEE_STRICTNESS);
	nativeFlag |= (bool(source & gxapi::eShaderCompileFlags::WARNINGS_AS_ERRORS) * D3DCOMPILE_WARNINGS_ARE_ERRORS);

	return nativeFlag;
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


D3D12_RANGE native_cast(gxapi::MemoryRange source) {
	D3D12_RANGE result;

	result.Begin = source.begin;
	result.End = source.end;

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
	D3D12_RESOURCE_DESC result = {};

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
	else {
		assert(false);
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
	result.NumDescriptors = static_cast<UINT>(source.numDescriptors);
	result.Flags = source.isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result.NodeMask = 0;

	return result;
}


D3D12_BLEND_DESC native_cast(gxapi::BlendState source) {
	D3D12_BLEND_DESC result;

	result.AlphaToCoverageEnable = source.alphaToCoverage;
	result.IndependentBlendEnable = source.independentBlending;

	size_t renderTargetCount = sizeof(result.RenderTarget) / sizeof(result.RenderTarget[0]);
	for (size_t i = 0; i < renderTargetCount; i++) {
		result.RenderTarget[i] = native_cast(source.multiTarget[i]);
	}

	return result;
}


D3D12_RENDER_TARGET_BLEND_DESC native_cast(gxapi::RenderTargetBlendState source) {
	D3D12_RENDER_TARGET_BLEND_DESC result;

	result.BlendEnable = source.enableBlending;
	result.LogicOpEnable = source.enableLogicOp;
	result.SrcBlend = native_cast(source.shaderColorFactor);
	result.DestBlend = native_cast(source.targetColorFactor);
	result.BlendOp = native_cast(source.colorOperation);
	result.SrcBlendAlpha = native_cast(source.shaderAlphaFactor);
	result.DestBlendAlpha = native_cast(source.targetAlphaFactor);
	result.BlendOpAlpha = native_cast(source.alphaOperation);
	result.LogicOp = native_cast(source.logicOperation);
	result.RenderTargetWriteMask = native_cast(source.mask);

	return result;
}


D3D12_RASTERIZER_DESC native_cast(gxapi::RasterizerState source) {
	D3D12_RASTERIZER_DESC result;

	result.FillMode = native_cast(source.fillMode);
	result.CullMode = native_cast(source.cullMode);
	result.FrontCounterClockwise = frontFaceIsCCW;
	result.DepthBias = source.depthBias;
	result.DepthBiasClamp = source.depthBiasClamp;
	result.SlopeScaledDepthBias = source.slopeScaledDepthBias;
	result.DepthClipEnable = source.depthClipEnabled;
	result.MultisampleEnable = source.multisampleEnabled;
	result.AntialiasedLineEnable = source.lineAntialiasingEnabled;
	result.ForcedSampleCount = source.forcedSampleCount;
	result.ConservativeRaster = native_cast(source.conservativeRasterization);

	return result;
}


D3D12_DEPTH_STENCIL_DESC native_cast(gxapi::DepthStencilState source) {
	D3D12_DEPTH_STENCIL_DESC result;

	result.DepthEnable = source.enableDepthTest;
	result.DepthWriteMask = source.enableDepthStencilWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	result.DepthFunc = native_cast(source.depthFunc);
	result.StencilEnable = source.enableStencilTest;
	result.StencilReadMask = source.stencilReadMask;
	result.StencilWriteMask = source.stencilWriteMask;

	static_assert(frontFaceIsCCW == true, "Front face expected to be counter clockwise");
	result.FrontFace = native_cast(source.ccwFace);
	result.BackFace = native_cast(source.cwFace);

	return result;
}


D3D12_DEPTH_STENCILOP_DESC native_cast(gxapi::DepthStencilState::FaceOperations source) {
	D3D12_DEPTH_STENCILOP_DESC result;

	result.StencilFunc = native_cast(source.stencilFunc);
	result.StencilPassOp = native_cast(source.stencilOpOnPass);
	result.StencilFailOp = native_cast(source.stencilOpOnStencilFail);
	result.StencilDepthFailOp = native_cast(source.stencilOpOnDepthFail);

	return result;
}


D3D12_INPUT_ELEMENT_DESC native_cast(gxapi::InputElementDesc source) {
	D3D12_INPUT_ELEMENT_DESC result;

	result.SemanticName = source.semanticName;
	result.SemanticIndex = source.semanticIndex;
	result.Format = native_cast(source.format);
	result.InputSlot = source.inputSlot;
	result.AlignedByteOffset = source.offset;
	result.InputSlotClass = native_cast(source.classifiacation);
	result.InstanceDataStepRate = source.instanceDataStepRate;

	return result;
}


D3D12_CONSTANT_BUFFER_VIEW_DESC native_cast(gxapi::ConstantBufferViewDesc source) {
	D3D12_CONSTANT_BUFFER_VIEW_DESC result;

	result.BufferLocation = native_cast_ptr(source.gpuVirtualAddress);
	result.SizeInBytes = static_cast<UINT>(source.sizeInBytes);

	return result;
}


D3D12_DEPTH_STENCIL_VIEW_DESC native_cast(gxapi::DepthStencilViewDesc source) {
	D3D12_DEPTH_STENCIL_VIEW_DESC result;

	result.Format = native_cast(source.format);
	result.ViewDimension = native_cast(source.dimension);
	result.Flags = native_cast(source.flags);

	switch (result.ViewDimension) {
	case D3D12_DSV_DIMENSION_UNKNOWN:
		assert(false);
		break;
	case D3D12_DSV_DIMENSION_TEXTURE1D:
		result.Texture1D = native_cast(source.tex1D);
		break;
	case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
		result.Texture1DArray = native_cast(source.tex1DArray);
		break;
	case D3D12_DSV_DIMENSION_TEXTURE2D:
		result.Texture2D = native_cast(source.tex2D);
		break;
	case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
		result.Texture2DArray = native_cast(source.tex2DArray);
		break;
	case D3D12_DSV_DIMENSION_TEXTURE2DMS:
		result.Texture2DMS = native_cast(source.texMS2D);
		break;
	case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
		result.Texture2DMSArray = native_cast(source.texMS2DArray);
		break;
	default:
		assert(false);
		break;
	}

	return result;
}


D3D12_RENDER_TARGET_VIEW_DESC native_cast(gxapi::RenderTargetViewDesc source) {
	D3D12_RENDER_TARGET_VIEW_DESC result;

	result.Format = native_cast(source.format);
	result.ViewDimension = native_cast(source.dimension);

	switch (result.ViewDimension) {
	case D3D12_RTV_DIMENSION_UNKNOWN:
		assert(false);
		break;
	case D3D12_RTV_DIMENSION_BUFFER:
		result.Buffer = native_cast(source.buffer);
		break;
	case D3D12_RTV_DIMENSION_TEXTURE1D:
		result.Texture1D = native_cast(source.tex1D);
		break;
	case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
		result.Texture1DArray = native_cast(source.tex1DArray);
		break;
	case D3D12_RTV_DIMENSION_TEXTURE2D:
		result.Texture2D = native_cast(source.tex2D);
		break;
	case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
		result.Texture2DArray = native_cast(source.tex2DArray);
		break;
	case D3D12_RTV_DIMENSION_TEXTURE2DMS:
		result.Texture2DMS = native_cast(source.texMS2D);
		break;
	case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
		result.Texture2DMSArray = native_cast(source.texMS2DArray);
		break;
	case D3D12_RTV_DIMENSION_TEXTURE3D:
		result.Texture3D = native_cast(source.tex3D);
		break;
	default:
		assert(false);
		break;
	}

	return result;
}


D3D12_SHADER_RESOURCE_VIEW_DESC native_cast(gxapi::ShaderResourceViewDesc source) {
	D3D12_SHADER_RESOURCE_VIEW_DESC result;

	result.Format = native_cast(source.format);
	result.ViewDimension = native_cast(source.dimension);
	result.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (result.ViewDimension) {
	case D3D12_SRV_DIMENSION_UNKNOWN:
		assert(false);
		break;
	case D3D12_SRV_DIMENSION_BUFFER:
		result.Buffer = native_cast(source.buffer);
		break;
	case D3D12_SRV_DIMENSION_TEXTURE1D:
		result.Texture1D = native_cast(source.tex1D);
		break;
	case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
		result.Texture1DArray = native_cast(source.tex1DArray);
		break;
	case D3D12_SRV_DIMENSION_TEXTURE2D:
		result.Texture2D = native_cast(source.tex2D);
		break;
	case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
		result.Texture2DArray = native_cast(source.tex2DArray);
		break;
	case D3D12_SRV_DIMENSION_TEXTURE2DMS:
		result.Texture2DMS = native_cast(source.texMS2D);
		break;
	case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
		result.Texture2DMSArray = native_cast(source.texMS2DArray);
		break;
	case D3D12_SRV_DIMENSION_TEXTURE3D:
		result.Texture3D = native_cast(source.tex3D);
		break;
	case D3D12_SRV_DIMENSION_TEXTURECUBE:
		result.TextureCube = native_cast(source.texCube);
		break;
	case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
		result.TextureCubeArray = native_cast(source.texCubeArray);
		break;
	default:
		assert(false);
		break;
	}

	return result;
}


D3D12_UNORDERED_ACCESS_VIEW_DESC native_cast(gxapi::UnorderedAccessViewDesc source) {
	D3D12_UNORDERED_ACCESS_VIEW_DESC result;

	result.Format = native_cast(source.format);
	result.ViewDimension = native_cast(source.dimension);

	switch (result.ViewDimension) {
		case D3D12_UAV_DIMENSION_UNKNOWN:
			assert(false);
			break;
		case D3D12_UAV_DIMENSION_BUFFER:
			result.Buffer = native_cast(source.buffer);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE1D:
			result.Texture1D = native_cast(source.tex1D);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
			result.Texture1DArray = native_cast(source.tex1DArray);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE2D:
			result.Texture2D = native_cast(source.tex2D);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
			result.Texture2DArray = native_cast(source.tex2DArray);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE3D:
			result.Texture3D = native_cast(source.tex3D);
			break;
		default:
			assert(false);
			break;
	}

	return result;
}


DXGI_SWAP_CHAIN_DESC native_cast(gxapi::SwapChainDesc source) {
	DXGI_SWAP_CHAIN_DESC result;

	//TODO some values are set to a constant that might need to be customizable
	result.BufferDesc.Width = source.width;
	result.BufferDesc.Height = source.height;
	result.BufferDesc.RefreshRate.Numerator = 60; 
	result.BufferDesc.RefreshRate.Denominator = 1;
	result.BufferDesc.Format = native_cast(source.format);
	result.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	result.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;

	result.SampleDesc.Count = source.multisampleCount;
	result.SampleDesc.Quality = source.multiSampleQuality;

	result.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	result.BufferCount = source.numBuffers;
	result.OutputWindow = source.targetWindow;
	result.Windowed = !source.isFullScreen;
	result.SwapEffect = source.multisampleCount > 1 ? DXGI_SWAP_EFFECT_DISCARD : DXGI_SWAP_EFFECT_FLIP_DISCARD;

	result.Flags = 0;

	return result;
}


D3D12_TEX1D_DSV native_cast(gxapi::DsvTexture1D source) {
	D3D12_TEX1D_DSV  result;

	result.MipSlice = source.firstMipLevel;

	return result;
}


D3D12_TEX1D_ARRAY_DSV native_cast(gxapi::DsvTexture1DArray source) {
	D3D12_TEX1D_ARRAY_DSV result;

	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;
	result.MipSlice = source.firstMipLevel;

	return result;
}


D3D12_TEX2D_DSV native_cast(gxapi::DsvTexture2D source) {
	D3D12_TEX2D_DSV result;

	result.MipSlice = source.firstMipLevel;

	return result;
}


D3D12_TEX2D_ARRAY_DSV native_cast(gxapi::DsvTexture2DArray source) {
	D3D12_TEX2D_ARRAY_DSV result;

	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;
	result.MipSlice = source.firstMipLevel;

	return result;
}


D3D12_TEX2DMS_DSV native_cast(gxapi::DsvTextureMultisampled2D source) {
	D3D12_TEX2DMS_DSV result = {};

	return result;
}


D3D12_TEX2DMS_ARRAY_DSV native_cast(gxapi::DsvTextureMultisampled2DArray source) {
	D3D12_TEX2DMS_ARRAY_DSV result;

	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;

	return result;
}


D3D12_BUFFER_RTV native_cast(gxapi::RtvBuffer source) {
	D3D12_BUFFER_RTV result;

	result.NumElements = source.numElements;
	result.FirstElement = source.firstElement;

	return result;
}

D3D12_TEX1D_RTV native_cast(gxapi::RtvTexture1D source) {
	D3D12_TEX1D_RTV result;

	result.MipSlice = source.firstMipLevel;

	return result;
}


D3D12_TEX1D_ARRAY_RTV native_cast(gxapi::RtvTexture1DArray source){
	D3D12_TEX1D_ARRAY_RTV result;

	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;
	result.MipSlice = source.firstMipLevel;

	return result;
}


D3D12_TEX2D_RTV native_cast(gxapi::RtvTexture2D source) {
	D3D12_TEX2D_RTV result;

	result.PlaneSlice = source.planeIndex;
	result.MipSlice = source.firstMipLevel;

	return result;
}


D3D12_TEX2D_ARRAY_RTV native_cast(gxapi::RtvTexture2DArray source) {
	D3D12_TEX2D_ARRAY_RTV result;

	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;
	result.PlaneSlice = source.planeIndex;
	result.MipSlice = source.firstMipLevel;

	return result;
}


D3D12_TEX2DMS_RTV native_cast(gxapi::RtvTextureMultisampled2D source) {
	D3D12_TEX2DMS_RTV result = {};
	return result;
}


D3D12_TEX2DMS_ARRAY_RTV native_cast(gxapi::RtvTextureMultisampled2DArray source) {
	D3D12_TEX2DMS_ARRAY_RTV result;

	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;

	return result;
}


D3D12_TEX3D_RTV native_cast(gxapi::RtvTexture3D source) {
	D3D12_TEX3D_RTV result;

	result.WSize = source.numDepthLevels;
	result.FirstWSlice = source.firstDepthIndex;
	result.MipSlice = source.firstMipLevel;

	return result;
}


D3D12_BUFFER_SRV native_cast(gxapi::SrvBuffer source) {
	D3D12_BUFFER_SRV result;

	result.NumElements = source.numElements;
	result.FirstElement = source.firstElement;
	result.StructureByteStride = source.structureStrideInBytes;
	result.Flags = source.isRaw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;

	return result;
}


D3D12_TEX1D_SRV native_cast(gxapi::SrvTexture1D source) {
	D3D12_TEX1D_SRV result;

	result.MipLevels = source.numMipLevels;
	result.MostDetailedMip = source.mostDetailedMip;
	result.ResourceMinLODClamp = source.mipLevelClamping;

	return result;
}


D3D12_TEX1D_ARRAY_SRV native_cast(gxapi::SrvTexture1DArray source) {
	D3D12_TEX1D_ARRAY_SRV result;

	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;
	result.MipLevels = source.numMipLevels;
	result.MostDetailedMip = source.mostDetailedMip;
	result.ResourceMinLODClamp = source.mipLevelClamping;

	return result;
}


D3D12_TEX2D_SRV native_cast(gxapi::SrvTexture2D source) {
	D3D12_TEX2D_SRV result;

	result.PlaneSlice = source.planeIndex;
	result.MipLevels = source.numMipLevels;
	result.MostDetailedMip = source.mostDetailedMip;
	result.ResourceMinLODClamp = source.mipLevelClamping;

	return result;
}


D3D12_TEX2D_ARRAY_SRV native_cast(gxapi::SrvTexture2DArray source) {
	D3D12_TEX2D_ARRAY_SRV result;

	result.PlaneSlice = source.planeIndex;
	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;
	result.MipLevels = source.numMipLevels;
	result.MostDetailedMip = source.mostDetailedMip;
	result.ResourceMinLODClamp = source.mipLevelClamping;

	return result;
}


D3D12_TEX2DMS_SRV native_cast(gxapi::SrvTextureMultisampled2D source) {
	D3D12_TEX2DMS_SRV result = {};

	return result;
}


D3D12_TEX2DMS_ARRAY_SRV native_cast(gxapi::SrvTextureMultisampled2DArray source) {
	D3D12_TEX2DMS_ARRAY_SRV result;

	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;

	return result;
}


D3D12_TEX3D_SRV native_cast(gxapi::SrvTexture3D source) {
	D3D12_TEX3D_SRV result;
	
	result.MipLevels = source.numMipLevels;
	result.MostDetailedMip = source.mostDetailedMip;
	result.ResourceMinLODClamp = source.mipLevelClamping;

	return result;
}


D3D12_TEXCUBE_SRV native_cast(gxapi::SrvTextureCube source) {
	D3D12_TEXCUBE_SRV result;

	result.MipLevels = source.numMipLevels;
	result.MostDetailedMip = source.mostDetailedMip;
	result.ResourceMinLODClamp = source.mipLevelClamping;

	return result;
}


D3D12_TEXCUBE_ARRAY_SRV native_cast(gxapi::SrvTextureCubeArray source) {
	D3D12_TEXCUBE_ARRAY_SRV result;

	result.NumCubes = source.numCubes;
	result.First2DArrayFace = source.indexOfFirst2DTex;
	result.MipLevels = source.numMipLevels;
	result.MostDetailedMip = source.mostDetailedMip;
	result.ResourceMinLODClamp = source.mipLevelClamping;

	return result;
}


D3D12_BUFFER_UAV native_cast(gxapi::UavBuffer source) {
	D3D12_BUFFER_UAV result;

	result.CounterOffsetInBytes = source.countOffset;
	result.FirstElement = source.firstElement;
	result.NumElements = source.numElements;
	result.StructureByteStride = source.elementStride;
	result.Flags = source.raw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;

	return result;
}


D3D12_TEX1D_UAV native_cast(gxapi::UavTexture1D source) {
	D3D12_TEX1D_UAV result;

	result.MipSlice = source.mipLevel;

	return result;
}


D3D12_TEX1D_ARRAY_UAV native_cast(gxapi::UavTexture1DArray source) {
	D3D12_TEX1D_ARRAY_UAV result;

	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;
	result.MipSlice = source.mipLevel;

	return result;
}


D3D12_TEX2D_UAV	native_cast(gxapi::UavTexture2D source) {
	D3D12_TEX2D_UAV result;

	result.PlaneSlice = source.planeIndex;
	result.MipSlice = source.mipLevel;

	return result;
}


D3D12_TEX2D_ARRAY_UAV native_cast(gxapi::UavTexture2DArray source) {
	D3D12_TEX2D_ARRAY_UAV result;

	result.PlaneSlice = source.planeIndex;
	result.ArraySize = source.activeArraySize;
	result.FirstArraySlice = source.firstArrayElement;
	result.MipSlice = source.mipLevel;

	return result;
}


D3D12_TEX3D_UAV native_cast(gxapi::UavTexture3D source) {
	D3D12_TEX3D_UAV result;

	result.MipSlice = source.mipLevel;
	result.FirstWSlice = source.firstDepthLayer;
	result.WSize = source.depthSize;

	return result;
}





//---------------
//OTHER

D3D12_DESCRIPTOR_RANGE native_cast(gxapi::DescriptorRange source) {
	D3D12_DESCRIPTOR_RANGE result;

	result.BaseShaderRegister = source.baseShaderRegister;
	result.NumDescriptors = source.numDescriptors;
	result.OffsetInDescriptorsFromTableStart =
		source.offsetFromTableStart == gxapi::DescriptorRange::OFFSET_APPEND ?
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND : source.offsetFromTableStart;
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


D3D12_SHADER_BYTECODE native_cast(gxapi::ShaderByteCodeDesc source) {
	D3D12_SHADER_BYTECODE result;

	result.BytecodeLength = source.sizeOfByteCode;
	result.pShaderBytecode = source.shaderByteCode;

	return result;
}


D3D12_RESOURCE_BARRIER native_cast(gxapi::ResourceBarrier source) {
	D3D12_RESOURCE_BARRIER native{};
	native.Type = native_cast(source.type);
	native.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;


	switch (source.type) {
		case gxapi::eResourceBarrierType::TRANSITION:
			native.Transition.StateBefore = native_cast(source.transition.beforeState);
			native.Transition.StateAfter = native_cast(source.transition.afterState);
			native.Transition.Subresource =
				source.transition.subResource == gxapi::ALL_SUBRESOURCES ?
				D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES : source.transition.subResource;
			native.Transition.pResource = native_cast(source.transition.resource);
			native.Flags = native_cast(source.transition.splitMode);
			break;
		case gxapi::eResourceBarrierType::ALIASING:
			break;
		case gxapi::eResourceBarrierType::UAV:
			native.UAV.pResource = native_cast(source.uav.resource);
			break;
		default:
			break;
	}
	return native;
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
		result += eResourceFlags::ALLOW_RENDER_TARGET;
	}
	if ((source & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0) {
		result += eResourceFlags::ALLOW_DEPTH_STENCIL;
	}
	if ((source & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0) {
		result += eResourceFlags::ALLOW_UNORDERED_ACCESS;
	}
	if ((source & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) != 0) {
		result += eResourceFlags::DENY_SHADER_RESOURCE;
	}
	if ((source & D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER) != 0) {
		result += eResourceFlags::ALLOW_CROSS_ADAPTER;
	}
	if ((source & D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS) != 0) {
		result += eResourceFlags::ALLOW_SIMULTANEOUS_ACCESS;
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

gxapi::eDescriptorHeapType native_cast(D3D12_DESCRIPTOR_HEAP_TYPE source) {
	using gxapi::eDescriptorHeapType;

	switch (source) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return eDescriptorHeapType::CBV_SRV_UAV;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return eDescriptorHeapType::SAMPLER;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return eDescriptorHeapType::RTV;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return eDescriptorHeapType::DSV;
	default:
		assert(false);
	}

	return eDescriptorHeapType{};
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


gxapi::SwapChainDesc native_cast(DXGI_SWAP_CHAIN_DESC source) {
	gxapi::SwapChainDesc result;

	result.width = source.BufferDesc.Width;
	result.height = source.BufferDesc.Height;
	result.format = native_cast(source.BufferDesc.Format);
	result.multisampleCount = source.SampleDesc.Count;
	result.multiSampleQuality = source.SampleDesc.Quality;
	result.numBuffers = source.BufferCount;
	result.targetWindow = source.OutputWindow;
	result.isFullScreen = !source.Windowed;

	return result;
}


} // namespace gxapi_dx12
} // namespace inl
