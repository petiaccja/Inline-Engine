#include "ClearColor.hpp"
#include "GraphicsEngine_LL/Nodes/NodeUtility.hpp"
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include "GraphicsEngine_LL/AutoRegisterNode.hpp"


namespace inl::gxeng::nodes {

INL_REGISTER_GRAPHICS_NODE(ClearColor)


void ClearColor::Setup(SetupContext& context) {
	Texture2D input = GetInput<0>().Get();

	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_rtv = context.CreateRtv(input, input.GetFormat(), rtvDesc);

	GetOutput<0>().Set(input);
}

void ClearColor::Execute(RenderContext& context) {
	auto& list = context.AsGraphics();

	Vec4 clearValue = GetInput<1>().Get();
	list.SetResourceState(m_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	list.ClearRenderTarget(m_rtv, {clearValue.x, clearValue.y, clearValue.z, clearValue.w});
	m_rtv = {};
}

const std::string& ClearColor::GetInputName(size_t index) const {
	static const std::array<std::string, 2> names = {
		"ColorIn",
		"ClearValue",
	};
	return names[index];
}
const std::string& ClearColor::GetOutputName(size_t index) const {
	static const std::array<std::string, 1> names = {
		"ColorOut",
	};
	return names[index];
}

}
