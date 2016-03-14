#pragma once


namespace inl {
namespace gxapi {


class ICommandList {
public:
	virtual ~ICommandList() = default;

	virtual eCommandListType GetType() = 0;
};


} // namespace gxapi
} // namespace inl
