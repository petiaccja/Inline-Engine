#pragma once

#include "PipelineState.hpp"
#include "Resource.hpp"
#include "CommandAllocator.hpp"
#include "RootSignature.hpp"
#include "GraphicsCommandList.hpp"
#include "Fence.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {


ID3D12PipelineState* native_cast(gxapi::IPipelineState* source);

ID3D12Resource* native_cast(gxapi::IResource* source);

ID3D12CommandAllocator* native_cast(gxapi::ICommandAllocator* source);

ID3D12GraphicsCommandList* native_cast(gxapi::IGraphicsCommandList* source);

ID3D12RootSignature* native_cast(gxapi::IRootSignature* source);

ID3D12Fence* native_cast(gxapi::IFence* source);

D3D12_PRIMITIVE_TOPOLOGY native_cast(ePrimitiveTopology source);

D3D12_VIEWPORT native_cast(Viewport const & source);

D3D12_RECT native_cast(inl::Rectangle const & source);

DXGI_FORMAT native_cast(eFormat source);

}
}
