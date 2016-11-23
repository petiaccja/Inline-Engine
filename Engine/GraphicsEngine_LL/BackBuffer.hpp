#pragma once

#include "MemoryObject.hpp"
#include "ResourceView.hpp"


namespace inl {
namespace gxeng {


class BackBuffer : public Texture2D {
public:
	BackBuffer(DescriptorReference&& descRef, gxapi::RenderTargetViewDesc rtvDesc, MemoryObjDesc&& objDesc);

	RenderTargetView& GetView();

protected:
	RenderTargetView m_RTV;
};


} // namespace gxeng
} // namespace inl
