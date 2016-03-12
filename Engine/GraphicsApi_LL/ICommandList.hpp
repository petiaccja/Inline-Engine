#pragma once


namespace inl {
namespace gxapi {


enum class eCommandListType{
	COPY,
	COMPUTE,
	GRAPHICS,
	BUNDLE,
};


class ICommandList {
public:
	virtual eCommandListType GetType() = 0;
};


} // namespace gxapi
} // namespace inl
