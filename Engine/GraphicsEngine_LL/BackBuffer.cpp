#include "BackBuffer.hpp"


namespace inl {
namespace gxeng {


BackBuffer::BackBuffer(DescriptorReference&& descRef, gxapi::RenderTargetViewDesc rtvDesc, MemoryObjDesc&& objDesc) :
	Texture2D(std::move(objDesc)),
	m_RTV(*this, std::move(descRef), rtvDesc)
{}


RenderTargetView& BackBuffer::GetView() {
	return m_RTV;
}


} // namespace gxeng
} // namespace inl
