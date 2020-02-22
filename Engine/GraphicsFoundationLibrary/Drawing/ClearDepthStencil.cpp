#include "ClearDepthStencil.hpp"

#include "GraphicsEngine_LL/AutoRegisterNode.hpp"
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

namespace inl::gxeng::nodes {

INL_REGISTER_GRAPHICS_NODE(ClearDepthStencil)


void ClearDepthStencil::Setup(SetupContext& context) {
	Texture2D input = GetInput<0>().Get();

	const gxapi::eFormat currDepthStencilFormat = FormatAnyToDepthStencil(input.GetFormat());
	gxapi::DsvTexture2DArray dsvDesc;
	dsvDesc.activeArraySize = 1;
	dsvDesc.firstArrayElement = 0;
	dsvDesc.firstMipLevel = 0;
	m_dsv = context.CreateDsv(input, currDepthStencilFormat, dsvDesc);

	GetOutput<0>().Set(input);
}

void ClearDepthStencil::Execute(RenderContext& context) {
	auto& list = context.AsGraphics();

	float depthValue = GetInput<1>().Get();
	uint8_t stencilValue = GetInput<2>().Get();
	bool clearDepth = GetInput<3>().Get();
	bool clearStencil = GetInput<4>().Get();
	list.SetResourceState(m_dsv.GetResource(), gxapi::eResourceState::DEPTH_WRITE);
	list.ClearDepthStencil(m_dsv, depthValue, stencilValue, 0, nullptr, clearDepth, clearStencil);
	m_dsv = {};
}

const std::string& ClearDepthStencil::GetInputName(size_t index) const {
	static const std::array<std::string, 5> names = {
		"DepthStencilIn",
		"DepthValue",
		"StencilValue"
		"ClearDepth"
		"ClearStencil"
	};
	return names[index];
}
const std::string& ClearDepthStencil::GetOutputName(size_t index) const {
	static const std::array<std::string, 1> names = {
		"DepthStencilOut",
	};
	return names[index];
}

} // namespace inl::gxeng::nodes