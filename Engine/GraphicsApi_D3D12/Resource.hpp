#pragma once

#include "../GraphicsApi_LL/IResource.hpp"

#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class Resource : public gxapi::IResource {
public:
	ID3D12Resource* GetNative();

private:
	ComPtr<ID3D12Resource> m_native;
};

}
}
