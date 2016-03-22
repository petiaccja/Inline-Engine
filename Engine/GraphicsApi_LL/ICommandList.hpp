#pragma once

#include "Common.hpp"

namespace inl {
namespace gxapi {


class ICommandList {
public:
	virtual ~ICommandList() = default;

	virtual eCommandListType GetType() const = 0;
};


} // namespace gxapi
} // namespace inl
