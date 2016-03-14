#pragma once

#include "../GraphicsApi_LL/ICommandList.hpp"

#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {


class CommandList : public gxapi::ICommandList {
public:
	~CommandList() = default;
	ID3D12CommandList* GetNativeGenericList() = 0;

	gxapi::eCommandListType GetType() override;
};


}
}