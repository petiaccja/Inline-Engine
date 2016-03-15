#include "CommandList.hpp"

#include "NativeCast.hpp"
#include "../GraphicsApi_LL/Common.hpp"
#include <cassert>

namespace inl {
namespace gxapi_dx12 {

gxapi::eCommandListType CommandList::GetType() {
	ID3D12CommandList* native = GetNativeGenericList();

	return native_cast(native->GetType());
}


} // namespace gxapi_dx12
} // namespace inl
