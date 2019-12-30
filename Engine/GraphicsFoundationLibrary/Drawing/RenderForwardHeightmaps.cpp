#include "RenderForwardHeightmaps.hpp"

#include <BaseLibrary/Range.hpp>
#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>


namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(RenderForwardHeightmaps)


struct VsConstants {
	Mat44_Packed world;
	Mat44_Packed worldViewProj;
	Mat44_Packed worldViewProjDer;
	alignas(16) Vec3_Packed direction;
	float magnitude;
	float offset;
};

struct PsConstants {
	alignas(16) Vec3_Packed lightDir;
	alignas(16) Vec3_Packed lightColor;
};


RenderForwardHeightmaps::RenderForwardHeightmaps()
	: m_psoCache({}, {}) {}


void RenderForwardHeightmaps::Reset() {
	m_rtv = {};
	m_dsv = {};
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}


void RenderForwardHeightmaps::Setup(SetupContext& context) {
	auto renderTarget = GetInput<0>().Get();
	auto depthTarget = GetInput<1>().Get();

	CreateRenderTargetViews(context, renderTarget, depthTarget);
	//m_psoCache.SetTextureFormats(renderTarget.GetFormat(), depthTarget.GetFormat());

	GetOutput<0>().Set(renderTarget);
	GetOutput<1>().Set(depthTarget);
}


void RenderForwardHeightmaps::Execute(RenderContext& context) {
	// COPY CODE FROM FW RENDER SIMPLE
}



const std::string& RenderForwardHeightmaps::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
		"Camera",
		"Entities",
		"Lights",
	};
	return names[index];
}


const std::string& RenderForwardHeightmaps::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
	};
	return names[index];
}


void RenderForwardHeightmaps::CreateRenderTargetViews(SetupContext& context, const Texture2D& rt, const Texture2D& ds) {
	if (!m_rtv || rt != m_rtv.GetResource()) {
		gxapi::RtvTexture2DArray desc;
		desc.activeArraySize = 1;
		desc.firstArrayElement = 0;
		desc.firstMipLevel = 0;
		desc.planeIndex = 0;

		m_rtv = context.CreateRtv(rt, rt.GetFormat(), desc);
	}
	if (!m_dsv || ds != m_dsv.GetResource()) {
		gxapi::DsvTexture2DArray desc;
		desc.activeArraySize = 1;
		desc.firstArrayElement = 0;
		desc.firstMipLevel = 0;

		m_dsv = context.CreateDsv(ds, ds.GetFormat(), desc);
	}
}


} // namespace inl::gxeng::nodes
