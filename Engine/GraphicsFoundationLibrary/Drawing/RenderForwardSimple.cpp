#include "RenderForwardSimple.hpp"

#include "../Helpers/PipelineSetupUtility.hpp"

#include <BaseLibrary/Range.hpp>
#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>


namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(RenderForwardSimple)


struct VsConstants {
	Mat44_Packed world;
	Mat44_Packed viewProj;
	Mat44_Packed worldViewProjDer;
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

static const std::vector shaderBindParams = {
	vsConstantsBind,
	psConstantsBind,
};

static const PipelineStateTemplate psoTemplate = [] {
	PipelineStateTemplate psoTemplate;
	psoTemplate.vsFileName = "RenderForwardSimple.hlsl";
	psoTemplate.psFileName = "RenderForwardSimple.hlsl";

	psoTemplate.rasterization = gxapi::RasterizerState{ gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL };
	psoTemplate.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

	psoTemplate.depthStencilState = gxapi::DepthStencilState{ .enableDepthTest = true, .enableDepthStencilWrite = true };
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


RenderForwardSimple::RenderForwardSimple()
	: m_psoCache(psoTemplate, shaderBindParams) {}


void RenderForwardSimple::Reset() {
	m_rtv = {};
	m_dsv = {};
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}


void RenderForwardSimple::Setup(SetupContext& context) {
	const auto renderTarget = GetInput<0>().Get();
	const auto depthTarget = GetInput<1>().Get();

	CreateRenderTargetViews(context, renderTarget, depthTarget);
	UpdatePsoCache(renderTarget, depthTarget);

	GetOutput<0>().Set(renderTarget);
	GetOutput<1>().Set(depthTarget);
}


void RenderForwardSimple::Execute(RenderContext& context) {
	auto renderTarget = GetInput<0>().Get();
	auto depthTarget = GetInput<1>().Get();

	// Request command list.
	GraphicsCommandList& commandList = context.AsGraphics();

	// Set up the render pipeline.
	const int screenWidth = int(m_rtv.GetResource().GetWidth());
	const int screenHeight = int(m_rtv.GetResource().GetHeight());
	const RenderTargetView2D* renderTargets[] = { &m_rtv };
	const gxapi::Rectangle scissorRect = ScissorRect(screenWidth, screenHeight);
	const gxapi::Viewport viewport = Viewport(screenWidth, screenHeight);

	commandList.SetResourceState(m_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_dsv.GetResource(), gxapi::eResourceState::DEPTH_WRITE);
	commandList.SetRenderTargets(1, renderTargets, &m_dsv);
	commandList.SetScissorRects(1, &scissorRect);
	commandList.SetViewports(1, &viewport);
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
	commandList.SetStencilRef(1);

	// Render entities.
	RenderEntities(context, commandList);
}


const std::string& RenderForwardSimple::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
		"Camera",
		"Entities",
		"Lights",
	};
	return names[index];
}


const std::string& RenderForwardSimple::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
	};
	return names[index];
}


void RenderForwardSimple::CreateRenderTargetViews(SetupContext& context, const Texture2D& rt, const Texture2D& ds) {
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


void RenderForwardSimple::UpdatePsoCache(const Texture2D& renderTarget, const Texture2D& depthTarget) {
	if (m_psoCache.GetTemplate().renderTargetFormats[0] != renderTarget.GetFormat()
		|| m_psoCache.GetTemplate().depthStencilFormat != depthTarget.GetFormat()) {
		auto psoTemplateFmt = psoTemplate;
		psoTemplateFmt.renderTargetFormats[0] = renderTarget.GetFormat();
		psoTemplateFmt.depthStencilFormat = depthTarget.GetFormat();
		m_psoCache.Reset(psoTemplateFmt, shaderBindParams);
	}
}


void RenderForwardSimple::RenderEntities(RenderContext& context, GraphicsCommandList& commandList) {
	const auto camera = GetInput<2>().Get();
	const auto entities = GetInput<3>().Get();
	auto directionalLight = GetInput<4>().Get();

	const Mat44 view = camera->GetViewMatrix();
	const Mat44 proj = camera->GetProjectionMatrix();
	const Mat44 viewProj = view * proj;

	PsConstants psConstants{
		.lightDir = directionalLight->Size() > 0 ? (*directionalLight->begin())->GetDirection() : Vec3{ 0, 0, -1 },
		.lightColor = directionalLight->Size() > 0 ? (*directionalLight->begin())->GetColor() : Vec3{ 0, 0, 0 }
	};

	for (auto& entity : *entities) {
		if (!entity->GetMesh() || !entity->GetMaterial()) {
			continue;
		}

		VsConstants vsConstants;
		const Mesh& mesh = static_cast<const Mesh&>(*entity->GetMesh());
		const Material& material = static_cast<const Material&>(*entity->GetMaterial());

		const PipelineStateConfig& stateDesc = m_psoCache.GetConfig(context, mesh, material);

		const Mat44 world = entity->Transform().GetMatrix();
		vsConstants.world = world;
		vsConstants.viewProj = world * viewProj;
		vsConstants.worldViewProjDer = Mat44::Zero(); // TODO

		stateDesc.BindPipeline(commandList);
		commandList.BindGraphics(vsConstantsBind.parameter, &vsConstants, sizeof(vsConstants));
		commandList.BindGraphics(psConstantsBind.parameter, &psConstants, sizeof(psConstants));
		stateDesc.BindMaterial(commandList, material);

		BindMeshBuffers(commandList, mesh);

		commandList.DrawIndexedInstanced((unsigned)mesh.GetIndexBuffer().GetIndexCount(), 0, 0, 1, 0);
	}
}



} // namespace inl::gxeng::nodes
