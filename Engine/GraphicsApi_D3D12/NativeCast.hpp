#pragma once

#include "PipelineState.hpp"
#include "Resource.hpp"
#include "CommandAllocator.hpp"
#include "CommandQueue.hpp"
#include "RootSignature.hpp"
#include "GraphicsCommandList.hpp"
#include "Fence.hpp"
#include "../GraphicsApi_LL/Common.hpp"

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

D3D12_PRIMITIVE_TOPOLOGY native_cast(gxapi::ePrimitiveTopology source);

D3D12_COMMAND_LIST_TYPE native_cast(gxapi::eCommandListType source);

INT native_cast(gxapi::eCommandQueuePriority source);

D3D12_VIEWPORT native_cast(gxapi::Viewport const & source);

D3D12_RECT native_cast(gxapi::Rectangle const & source);

D3D12_BOX native_cast(gxapi::Cube source);

DXGI_FORMAT native_cast(gxapi::eFormat source);


////////////////////////////////////////////////////////////
// FROM NATIVE
////////////////////////////////////////////////////////////

gxapi::eCommandListType native_cast(D3D12_COMMAND_LIST_TYPE source);

gxapi::eCommandQueuePriority native_cast(D3D12_COMMAND_QUEUE_PRIORITY source);

gxapi::eDesriptorHeapType native_cast(D3D12_DESCRIPTOR_HEAP_TYPE source);

} // namespace gxapi_dx12
} // namespace inl
