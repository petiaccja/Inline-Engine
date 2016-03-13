#include "Fence.hpp"

namespace inl {
namespace gxapi {


ID3D12Fence* Fence::GetNative()
{
	return m_native;
}


}
}
