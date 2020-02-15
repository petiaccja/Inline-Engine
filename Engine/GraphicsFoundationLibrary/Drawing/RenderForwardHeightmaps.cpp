#include "RenderForwardHeightmaps.hpp"

#include "../Helpers/PipelineSetupUtility.hpp"

#include <BaseLibrary/Range.hpp>
#include <GraphicsEngine/Exception.hpp>
#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>


namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(RenderForwardHeightmaps)


struct VsConstants {
	Mat44_Packed world;
	Mat44_Packed viewProj;
	Mat44_Packed worldViewProjDer;
	Vec3_Packed direction;
	float _padding01 = 0.0f;
	float magnitude;
	float offset;
	Vec2_Packed uvSize;
	Vec2_Packed screenSize;
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
	.shaderVisibility = gxapi::eShaderVisiblity::ALL
};

static const BindParameterDesc psConstantsBind{
	.parameter = BindParameter{ eBindParameterType::CONSTANT, 100, 0 },
	.constantSize = sizeof(PsConstants),
	.relativeAccessFrequency = 1.0f,
	.relativeChangeFrequency = 1.0f,
	.shaderVisibility = gxapi::eShaderVisiblity::PIXEL
};

static const BindParameterDesc vsHeightMapBind{
	.parameter = BindParameter{ eBindParameterType::TEXTURE, 0, 0 },
	.shaderVisibility = gxapi::eShaderVisiblity::ALL
};

static const gxapi::StaticSamplerDesc vsHeightmapSampler{
	.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR,
	.addressU = gxapi::eTextureAddressMode::CLAMP,
	.addressV = gxapi::eTextureAddressMode::CLAMP,
	.addressW = gxapi::eTextureAddressMode::CLAMP,
	.mipLevelBias = 0.f,
	.shaderRegister = 0,
	.registerSpace = 0,
	.shaderVisibility = gxapi::eShaderVisiblity::ALL
};

static const std::vector shaderBindParams = {
	vsConstantsBind,
	psConstantsBind,
	vsHeightMapBind,
};

static const std::vector staticSamplers = {
	vsHeightmapSampler,
};

static const PipelineStateTemplate psoTemplate = [] {
	PipelineStateTemplate psoTemplate;
	psoTemplate.vsFileName = "RenderForwardHeightmaps.hlsl";
	psoTemplate.hsFileName = "RenderForwardHeightmaps.hlsl";
	psoTemplate.dsFileName = "RenderForwardHeightmaps.hlsl";
	psoTemplate.psFileName = "RenderForwardHeightmaps.hlsl";

	psoTemplate.rasterization = gxapi::RasterizerState{ gxapi::eFillMode::WIREFRAME, gxapi::eCullMode::DRAW_ALL };
	psoTemplate.primitiveTopologyType = gxapi::ePrimitiveTopologyType::PATCH;

	psoTemplate.depthStencilState = gxapi::DepthStencilState{ .enableDepthTest = true, .enableDepthStencilWrite = true, .enableStencilTest = false };
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
	: m_psoCache(psoTemplate, shaderBindParams, staticSamplers) {}


void RenderForwardHeightmaps::Reset() {
	m_rtv = {};
	m_dsv = {};
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}


void RenderForwardHeightmaps::Setup(SetupContext& context) {
	const auto renderTarget = GetInput<0>().Get();
	const auto depthTarget = GetInput<1>().Get();

	CreateRenderTargetViews(context, renderTarget, depthTarget);
	UpdatePsoCache(renderTarget, depthTarget);

	GetOutput<0>().Set(renderTarget);
	GetOutput<1>().Set(depthTarget);
}


void RenderForwardHeightmaps::Execute(RenderContext& context) {
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
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::PATCHLIST_3);
	commandList.SetStencilRef(1);

	// Render entities.
	RenderEntities(context, commandList);
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


void RenderForwardHeightmaps::UpdatePsoCache(const Texture2D& renderTarget, const Texture2D& depthTarget) {
	if (m_psoCache.GetTemplate().renderTargetFormats[0] != renderTarget.GetFormat()
		|| m_psoCache.GetTemplate().depthStencilFormat != depthTarget.GetFormat()) {
		auto psoTemplateFmt = psoTemplate;
		psoTemplateFmt.renderTargetFormats[0] = renderTarget.GetFormat();
		psoTemplateFmt.depthStencilFormat = depthTarget.GetFormat();
		m_psoCache.Reset(psoTemplateFmt, shaderBindParams, staticSamplers);
	}
}


void RenderForwardHeightmaps::RenderEntities(RenderContext& context, GraphicsCommandList& commandList) {
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
		IsEntityValid(*entity);

		VsConstants vsConstants;
		const Mesh& mesh = static_cast<const Mesh&>(*entity->GetMesh());
		const Material& material = static_cast<const Material&>(*entity->GetMaterial());
		const Image& heightmap = static_cast<const Image&>(*entity->GetHeightmap());


		const PipelineStateConfig& stateDesc = m_psoCache.GetConfig(context, mesh, material);

		const Mat44 world = entity->Transform().GetMatrix();
		vsConstants.world = world;
		vsConstants.viewProj = viewProj;
		vsConstants.worldViewProjDer = Mat44::Zero(); // TODO
		vsConstants.direction = entity->GetDirection();
		vsConstants.magnitude = entity->GetMagnitude();
		vsConstants.offset = entity->GetOffset();
		vsConstants.uvSize = entity->GetUvSize();
		vsConstants.screenSize = { m_rtv.GetResource().GetWidth(), m_rtv.GetResource().GetHeight() };

		stateDesc.BindPipeline(commandList);
		commandList.BindGraphics(vsConstantsBind.parameter, &vsConstants, sizeof(vsConstants));
		commandList.BindGraphics(psConstantsBind.parameter, &psConstants, sizeof(psConstants));
		commandList.SetResourceState(heightmap.GetSrv().GetResource(), gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE);
		commandList.BindGraphics(vsHeightMapBind.parameter, heightmap.GetSrv());
		stateDesc.BindMaterial(commandList, material);

		BindMeshBuffers(commandList, mesh);

		commandList.DrawIndexedInstanced((unsigned)mesh.GetIndexBuffer().GetIndexCount(), 0, 0, 1, 0);
	}
}


void RenderForwardHeightmaps::IsEntityValid(const IHeightmapEntity& entity) {
	auto ptr2str = [](const void* p) {
		std::stringstream ss;
		ss << p;
		return ss.str();
	};

	if (!entity.GetMesh()) {
		throw InvalidEntityException{ "HeightmapEntity has no associated mesh.", ptr2str(&entity) };
	}
	if (!entity.GetMaterial()) {
		throw InvalidEntityException{ "HeightmapEntity has no associated material.", ptr2str(&entity) };
	}
	if (!entity.GetHeightmap()) {
		throw InvalidEntityException{ "HeightmapEntity has no associated heightmap image.", ptr2str(&entity) };
	}

	const Mesh& mesh = static_cast<const Mesh&>(*entity.GetMesh());
	if (!IsMeshValid(mesh)) {
		throw InvalidEntityException{ "HeightmapEntity must have mesh with POSITION, TEXCOORD, NORMAL and TANGENT attributes.", ptr2str(&entity) };
	}
}


bool RenderForwardHeightmaps::IsMeshValid(const Mesh& mesh) {
	auto& layout = mesh.GetLayout();
	bool hasPosition = false;
	bool hasTexcoord = false;
	bool hasNormal = false;
	bool hasTangent = false;
	for (auto stream : Range(layout.GetStreamCount())) {
		for (const auto& element : layout[stream]) {
			hasPosition = hasPosition || element.semantic == eVertexElementSemantic::POSITION;
			hasTexcoord = hasTexcoord || element.semantic == eVertexElementSemantic::TEX_COORD;
			hasNormal = hasNormal || element.semantic == eVertexElementSemantic::NORMAL;
			hasTangent = hasTangent || element.semantic == eVertexElementSemantic::TANGENT;
		}
	}
	return hasPosition && hasTexcoord && hasNormal && hasTangent;
}


} // namespace inl::gxeng::nodes
