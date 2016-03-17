#pragma once

#include "Common.hpp"

namespace inl {
namespace gxapi {

class IResource {
public:
	virtual ~IResource() = default;

	virtual ResourceDesc GetDesc() = 0;
};

} // namespace gxapi
} // namespace inl
