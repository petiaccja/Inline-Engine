#pragma once

#include "../GraphicsApi_LL/IResource.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {
	
class Resource : public gxapi::IResource {
public:
	ID3D12Resource* GetNative();

private:
	ID3D12Resource* m_native;
};

}
}
