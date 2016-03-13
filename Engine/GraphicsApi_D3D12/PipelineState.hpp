#pragma once

#include "../GraphicsApi_LL/IPipelineState.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

class PipelineState : public gxapi::IPipelineState {
public:
	
	ID3D12PipelineState* GetNative();

private:
	ID3D12PipelineState* m_native;
};

}
}
