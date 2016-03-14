#include "CommandList.hpp"

#include "../GraphicsApi_LL/Common.hpp"
#include <cassert>

namespace inl {
namespace gxapi_dx12 {

gxapi::eCommandListType CommandList::GetType() {
	ID3D12CommandList* native = GetNativeGenericList();

	auto type = native->GetType();
	switch (type) {
	case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		return gxapi::eCommandListType::BUNDLE;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return gxapi::eCommandListType::COMPUTE;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return gxapi::eCommandListType::COPY;
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return gxapi::eCommandListType::GRAPHICS;
	default:
		assert(false);
	}

	return gxapi::eCommandListType{};
}


} // namespace gxapi_dx12
} // namespace inl
