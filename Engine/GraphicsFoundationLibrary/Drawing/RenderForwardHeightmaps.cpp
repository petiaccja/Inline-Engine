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
	Vec3_Packed direction;
	float magnitude;
	float offset;
};

struct PsConstants {
	alignas(16) Vec3_Packed lightDir;
	alignas(16) Vec3_Packed lightColor;
};

static const BindParameterDesc vsConstantsBind{
	.parameter = BindParameter{ eBindParameterType::CONSTANT, 0, 0 },
	.constantSize = sizeof(VsConstants),
	.relativeAccessFrequency = 1.0f,
	.relativeChangeFrequency = 1.0f,
	.shaderVisibility = gxapi::eShaderVisiblity::VERTEX
};

static const BindParameterDesc psConstantsBind{
	.parameter = BindParameter{ eBindParameterType::CONSTANT, 100, 0 },
	.constantSize = sizeof(PsConstants),
	.relativeAccessFrequency = 1.0f,
	.relativeChangeFrequency = 1.0f,
	.shaderVisibility = gxapi::eShaderVisiblity::PIXEL
};

static const PipelineStateTemplate psoTemplate = [] {
	PipelineStateTemplate psoTemplate;
	psoTemplate.vsFileName = "RenderForwardHeightmap.hlsl";
	psoTemplate.psFileName = "RenderForwardHeightmap.hlsl";

	psoTemplate.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
	psoTemplate.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

	psoTemplate.depthStencilState = gxapi::DepthStencilState(true, true);
	psoTemplate.depthStencilState.depthFunc = gxapi::eComparisonFunction::LESS_EQUAL;
	psoTemplate.depthStencilState.enableStencilTest = true;
	psoTemplate.depthStencilState.stencilReadMask = 0;
	psoTemplate.depthStencilState.stencilWriteMask = ~uint8_t(0);
	psoTemplate.depthStencilState.ccwFace.stencilFunc = gxapi::eComparisonFunction::ALWAYS;
	psoTemplate.depthStencilState.ccwFace.stencilOpOnStencilFail = gxapi::eStencilOp::KEEP;
	psoTemplate.depthStencilState.ccwFace.stencilOpOnDepthFail = gxapi::eStencilOp::KEEP;
	psoTemplate.depthStencilState.ccwFace.stencilOpOnPass = gxapi::eStencilOp::REPLACE;
	psoTemplate.depthStencilState.cwFace = psoTemplate.depthStencilState.ccwFace;
	psoTemplate.depthStencilFormat = gxapi::eFormat::UNKNOWN;

	psoTemplate.numRenderTargets = 1;
	psoTemplate.renderTargetFormats[0] = gxapi::eFormat::UNKNOWN;

	return psoTemplate;
}();


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
	
	if (m_psoCache.GetTemplate().renderTargetFormats[0] != renderTarget.GetFormat()
		|| m_psoCache.GetTemplate().depthStencilFormat != depthTarget.GetFormat()) {
		auto psoTemplateFmt = psoTemplate;
		psoTemplateFmt.renderTargetFormats[0] = renderTarget.GetFormat();
		psoTemplateFmt.depthStencilFormat = depthTarget.GetFormat();
		m_psoCache.Reset({ psConstantsBind, vsConstantsBind }, psoTemplateFmt);
	}

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
