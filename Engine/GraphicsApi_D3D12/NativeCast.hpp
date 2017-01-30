#pragma once

#include "PipelineState.hpp"
#include "Resource.hpp"
#include "CommandAllocator.hpp"
#include "CommandQueue.hpp"
#include "RootSignature.hpp"
#include "DescriptorHeap.hpp"
#include "CommandList.hpp"
#include "Fence.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d12.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {

////////////////////////////////////////////////////////////
//CONSTANT
constexpr bool frontFaceIsCCW = true;


////////////////////////////////////////////////////////////
// SPECIAL
void* native_cast_ptr(std::uintptr_t source);

std::uintptr_t native_cast_ptr(const void* source);


////////////////////////////////////////////////////////////
// TO NATIVE
////////////////////////////////////////////////////////////

//---------------
//INTERFACE
ID3D12PipelineState* native_cast(gxapi::IPipelineState* source);

ID3D12Resource* native_cast(gxapi::IResource* source);

const ID3D12Resource* native_cast(const gxapi::IResource* source);

ID3D12CommandAllocator* native_cast(gxapi::ICommandAllocator* source);

ID3D12GraphicsCommandList* native_cast(gxapi::IGraphicsCommandList* source);

ID3D12RootSignature* native_cast(gxapi::IRootSignature* source);

ID3D12DescriptorHeap* native_cast(gxapi::IDescriptorHeap* source);

ID3D12Fence* native_cast(gxapi::IFence* source);

ID3D12CommandQueue* native_cast(gxapi::ICommandQueue* source);

//---------------
//ENUM
D3D12_SHADER_VISIBILITY native_cast(gxapi::eShaderVisiblity source);

D3D12_PRIMITIVE_TOPOLOGY native_cast(gxapi::ePrimitiveTopology source);

D3D12_COMMAND_LIST_TYPE native_cast(gxapi::eCommandListType source);

D3D12_DESCRIPTOR_HEAP_TYPE native_cast(gxapi::eDescriptorHeapType source);

D3D12_ROOT_PARAMETER_TYPE native_cast(gxapi::RootParameterDesc::eType source);

D3D12_DESCRIPTOR_RANGE_TYPE native_cast(gxapi::DescriptorRange::eType source);

D3D12_TEXTURE_ADDRESS_MODE native_cast(gxapi::eTextureAddressMode source);

D3D12_FILTER native_cast(gxapi::eTextureFilterMode source);

D3D12_COMPARISON_FUNC native_cast(gxapi::eComparisonFunction source);

INT native_cast(gxapi::eCommandQueuePriority source);

D3D12_HEAP_TYPE native_cast(gxapi::eHeapType source);

D3D12_CPU_PAGE_PROPERTY native_cast(gxapi::eCpuPageProperty source);

D3D12_MEMORY_POOL native_cast(gxapi::eMemoryPool source);

D3D12_STATIC_BORDER_COLOR native_cast(gxapi::eTextureBorderColor source);

DXGI_FORMAT native_cast(gxapi::eFormat source);

D3D12_TEXTURE_LAYOUT native_cast(gxapi::eTextureLayout source);

D3D12_RESOURCE_DIMENSION native_cast(gxapi::eTextueDimension source);

D3D12_BLEND native_cast(gxapi::eBlendOperand source);

D3D12_BLEND_OP native_cast(gxapi::eBlendOperation source);

D3D12_LOGIC_OP native_cast(gxapi::eBlendLogicOperation source);

D3D12_FILL_MODE native_cast(gxapi::eFillMode source);

D3D12_CULL_MODE native_cast(gxapi::eCullMode source);

D3D12_CONSERVATIVE_RASTERIZATION_MODE native_cast(gxapi::eConservativeRasterizationMode source);

D3D12_STENCIL_OP native_cast(gxapi::eStencilOp source);

D3D12_INDEX_BUFFER_STRIP_CUT_VALUE native_cast(gxapi::eTriangleStripCutIndex source);

D3D12_PRIMITIVE_TOPOLOGY_TYPE native_cast(gxapi::ePrimitiveTopologyType source);

D3D12_INPUT_CLASSIFICATION native_cast(gxapi::eInputClassification source);

D3D12_DSV_DIMENSION native_cast(gxapi::eDsvDimension source);

D3D12_RTV_DIMENSION native_cast(gxapi::eRtvDimension source);

D3D12_SRV_DIMENSION native_cast(gxapi::eSrvDimension source);

D3D12_UAV_DIMENSION native_cast(gxapi::eUavDimension source);

D3D12_RESOURCE_BARRIER_FLAGS native_cast(gxapi::eResourceBarrierSplit source);

D3D12_RESOURCE_BARRIER_TYPE native_cast(gxapi::eResourceBarrierType source);

//---------------
//FLAGS
D3D12_RESOURCE_FLAGS native_cast(gxapi::eResourceFlags source);

D3D12_HEAP_FLAGS native_cast(gxapi::eHeapFlags source);

D3D12_RESOURCE_STATES native_cast(gxapi::eResourceState source);

UINT8 native_cast(gxapi::eColorMask source);

D3D12_DSV_FLAGS native_cast(gxapi::eDsvFlags source);

UINT native_cast(gxapi::eShaderCompileFlags);

//---------------
//OBJECT
D3D12_VIEWPORT native_cast(gxapi::Viewport const & source);

D3D12_RECT native_cast(gxapi::Rectangle const & source);

D3D12_BOX native_cast(gxapi::Cube source);

D3D12_CLEAR_VALUE native_cast(gxapi::ClearValue source);

D3D12_RANGE native_cast(gxapi::MemoryRange source);

//---------------
//DESCRIPTOR
D3D12_ROOT_DESCRIPTOR native_cast(gxapi::RootDescriptor source);

D3D12_HEAP_PROPERTIES native_cast(gxapi::HeapProperties source);

D3D12_RESOURCE_DESC native_cast(gxapi::ResourceDesc source);

D3D12_STATIC_SAMPLER_DESC native_cast(gxapi::StaticSamplerDesc source);

D3D12_COMMAND_QUEUE_DESC native_cast(gxapi::CommandQueueDesc source);

D3D12_DESCRIPTOR_HEAP_DESC native_cast(gxapi::DescriptorHeapDesc source);

D3D12_BLEND_DESC native_cast(gxapi::BlendState source);

D3D12_RENDER_TARGET_BLEND_DESC native_cast(gxapi::RenderTargetBlendState source);

D3D12_RASTERIZER_DESC native_cast(gxapi::RasterizerState source);

D3D12_DEPTH_STENCIL_DESC native_cast(gxapi::DepthStencilState source);

D3D12_DEPTH_STENCILOP_DESC native_cast(gxapi::DepthStencilState::FaceOperations source);

D3D12_INPUT_ELEMENT_DESC native_cast(gxapi::InputElementDesc source);

D3D12_CONSTANT_BUFFER_VIEW_DESC native_cast(gxapi::ConstantBufferViewDesc source);

D3D12_DEPTH_STENCIL_VIEW_DESC native_cast(gxapi::DepthStencilViewDesc source);

D3D12_RENDER_TARGET_VIEW_DESC native_cast(gxapi::RenderTargetViewDesc source);

D3D12_SHADER_RESOURCE_VIEW_DESC native_cast(gxapi::ShaderResourceViewDesc source);

D3D12_UNORDERED_ACCESS_VIEW_DESC native_cast(gxapi::UnorderedAccessViewDesc source);

DXGI_SWAP_CHAIN_DESC native_cast(gxapi::SwapChainDesc source);

D3D12_TEX1D_DSV native_cast(gxapi::DsvTexture1D source);

D3D12_TEX1D_ARRAY_DSV native_cast(gxapi::DsvTexture1DArray source);

D3D12_TEX2D_DSV native_cast(gxapi::DsvTexture2D source);

D3D12_TEX2D_ARRAY_DSV native_cast(gxapi::DsvTexture2DArray source);

D3D12_TEX2DMS_DSV native_cast(gxapi::DsvTextureMultisampled2D source);

D3D12_TEX2DMS_ARRAY_DSV native_cast(gxapi::DsvTextureMultisampled2DArray source);

D3D12_BUFFER_RTV native_cast(gxapi::RtvBuffer source);

D3D12_TEX1D_RTV native_cast(gxapi::RtvTexture1D source);

D3D12_TEX1D_ARRAY_RTV native_cast(gxapi::RtvTexture1DArray source);

D3D12_TEX2D_RTV native_cast(gxapi::RtvTexture2D source);

D3D12_TEX2D_ARRAY_RTV native_cast(gxapi::RtvTexture2DArray source);

D3D12_TEX2DMS_RTV native_cast(gxapi::RtvTextureMultisampled2D source);

D3D12_TEX2DMS_ARRAY_RTV native_cast(gxapi::RtvTextureMultisampled2DArray source);

D3D12_TEX3D_RTV native_cast(gxapi::RtvTexture3D source);

D3D12_BUFFER_SRV native_cast(gxapi::SrvBuffer source);

D3D12_TEX1D_SRV native_cast(gxapi::SrvTexture1D source);

D3D12_TEX1D_ARRAY_SRV native_cast(gxapi::SrvTexture1DArray source);

D3D12_TEX2D_SRV	native_cast(gxapi::SrvTexture2D source);

D3D12_TEX2D_ARRAY_SRV native_cast(gxapi::SrvTexture2DArray source);

D3D12_TEX2DMS_SRV native_cast(gxapi::SrvTextureMultisampled2D source);

D3D12_TEX2DMS_ARRAY_SRV native_cast(gxapi::SrvTextureMultisampled2DArray source);

D3D12_TEX3D_SRV native_cast(gxapi::SrvTexture3D source);

D3D12_TEXCUBE_SRV native_cast(gxapi::SrvTextureCube source);

D3D12_TEXCUBE_ARRAY_SRV native_cast(gxapi::SrvTextureCubeArray source);

D3D12_BUFFER_UAV native_cast(gxapi::UavBuffer source);

D3D12_TEX1D_UAV native_cast(gxapi::UavTexture1D source);

D3D12_TEX1D_ARRAY_UAV native_cast(gxapi::UavTexture1DArray source);

D3D12_TEX2D_UAV	native_cast(gxapi::UavTexture2D source);

D3D12_TEX2D_ARRAY_UAV native_cast(gxapi::UavTexture2DArray source);

D3D12_TEX3D_UAV native_cast(gxapi::UavTexture3D source);

D3D12_RESOURCE_BARRIER native_cast(gxapi::ResourceBarrier source);


//---------------
//OTHER
D3D12_DESCRIPTOR_RANGE native_cast(gxapi::DescriptorRange source);

D3D12_ROOT_CONSTANTS native_cast(gxapi::RootConstant source);

D3D12_SHADER_BYTECODE native_cast(gxapi::ShaderByteCodeDesc source);


////////////////////////////////////////////////////////////
// FROM NATIVE
////////////////////////////////////////////////////////////

gxapi::eTextueDimension texture_dimension_cast(D3D12_RESOURCE_DIMENSION source);

gxapi::eFormat native_cast(DXGI_FORMAT source);

gxapi::eTextureLayout native_cast(D3D12_TEXTURE_LAYOUT source);

gxapi::eResourceFlags native_cast(D3D12_RESOURCE_FLAGS source);

gxapi::eCommandListType native_cast(D3D12_COMMAND_LIST_TYPE source);

gxapi::eCommandQueuePriority native_cast(D3D12_COMMAND_QUEUE_PRIORITY source);

gxapi::eDescriptorHeapType native_cast(D3D12_DESCRIPTOR_HEAP_TYPE source);

gxapi::CommandQueueDesc native_cast(D3D12_COMMAND_QUEUE_DESC source);

gxapi::DescriptorHeapDesc native_cast(D3D12_DESCRIPTOR_HEAP_DESC source);

gxapi::ResourceDesc native_cast(D3D12_RESOURCE_DESC source);

gxapi::SwapChainDesc native_cast(DXGI_SWAP_CHAIN_DESC source);


} // namespace gxapi_dx12
} // namespace inl
