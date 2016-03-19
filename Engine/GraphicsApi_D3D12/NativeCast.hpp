#pragma once

#include "PipelineState.hpp"
#include "Resource.hpp"
#include "CommandAllocator.hpp"
#include "CommandQueue.hpp"
#include "RootSignature.hpp"
#include "GraphicsCommandList.hpp"
#include "Fence.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

////////////////////////////////////////////////////////////
// TO NATIVE
////////////////////////////////////////////////////////////

ID3D12PipelineState* native_cast(gxapi::IPipelineState* source);

ID3D12Resource* native_cast(gxapi::IResource* source);

ID3D12CommandAllocator* native_cast(gxapi::ICommandAllocator* source);

ID3D12GraphicsCommandList* native_cast(gxapi::IGraphicsCommandList* source);

ID3D12RootSignature* native_cast(gxapi::IRootSignature* source);

ID3D12Fence* native_cast(gxapi::IFence* source);

D3D12_SHADER_VISIBILITY native_cast(gxapi::eShaderVisiblity source);

D3D12_PRIMITIVE_TOPOLOGY native_cast(gxapi::ePrimitiveTopology source);

D3D12_COMMAND_LIST_TYPE native_cast(gxapi::eCommandListType source);

D3D12_DESCRIPTOR_HEAP_TYPE native_cast(gxapi::eDesriptorHeapType source);

D3D12_ROOT_PARAMETER_TYPE native_cast(gxapi::RootParameterDesc::eType source);

D3D12_DESCRIPTOR_RANGE_TYPE native_cast(gxapi::DescriptorRange::eType source);

D3D12_TEXTURE_ADDRESS_MODE native_cast(gxapi::eTextureAddressMode source);

D3D12_FILTER native_cast(gxapi::eTextureFilterMode source);

D3D12_COMPARISON_FUNC native_cast(gxapi::eComparisonFunction source);

INT native_cast(gxapi::eCommandQueuePriority source);

D3D12_STATIC_BORDER_COLOR native_cast(gxapi::eTextureBorderColor source);

D3D12_VIEWPORT native_cast(gxapi::Viewport const & source);

D3D12_RECT native_cast(gxapi::Rectangle const & source);

D3D12_BOX native_cast(gxapi::Cube source);

DXGI_FORMAT native_cast(gxapi::eFormat source);

D3D12_DESCRIPTOR_RANGE native_cast(gxapi::DescriptorRange source);

D3D12_ROOT_CONSTANTS native_cast(gxapi::RootConstant source);

D3D12_ROOT_DESCRIPTOR native_cast(gxapi::RootDescriptor source);

D3D12_HEAP_PROPERTIES native_cast(gxapi::HeapProperties source);

D3D12_STATIC_SAMPLER_DESC native_cast(gxapi::StaticSamplerDesc source);

D3D12_COMMAND_QUEUE_DESC native_cast(gxapi::CommandQueueDesc source);

D3D12_DESCRIPTOR_HEAP_DESC native_cast(gxapi::DescriptorHeapDesc source);

////////////////////////////////////////////////////////////
// FROM NATIVE
////////////////////////////////////////////////////////////

gxapi::eTextueDimension texture_dimension_cast(D3D12_RESOURCE_DIMENSION source);

gxapi::eFormat native_cast(DXGI_FORMAT source);

gxapi::eTextureLayout native_cast(D3D12_TEXTURE_LAYOUT source);

gxapi::eResourceFlags native_cast(D3D12_RESOURCE_FLAGS source);

gxapi::eCommandListType native_cast(D3D12_COMMAND_LIST_TYPE source);

gxapi::eCommandQueuePriority native_cast(D3D12_COMMAND_QUEUE_PRIORITY source);

gxapi::eDesriptorHeapType native_cast(D3D12_DESCRIPTOR_HEAP_TYPE source);

gxapi::CommandQueueDesc native_cast(D3D12_COMMAND_QUEUE_DESC source);

gxapi::DescriptorHeapDesc native_cast(D3D12_DESCRIPTOR_HEAP_DESC source);

gxapi::ResourceDesc native_cast(D3D12_RESOURCE_DESC source);


} // namespace gxapi_dx12
} // namespace inl
