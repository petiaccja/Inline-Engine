#include "Node_Voxelization.hpp"

#include "NodeUtility.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../GraphicsCommandList.hpp"

#include <array>

namespace inl::gxeng::nodes {

const int voxelDimension = 256; //units
const float voxelSize = 0.16f; //meters
const Vec3 voxelOrigin = Vec3(voxelDimension * voxelSize * -0.5);
const Vec3 voxelCenter = Vec3(0.0f);

struct Uniforms
{
	Mat44_Packed model, viewProj, invView;
	Vec3_Packed voxelCenter; float voxelSize;
	Vec4_Packed farPlaneData0, farPlaneData1, wsCamPos;
	int voxelDimension; int inputMipLevel; int outputMipLevel; int dummy;
	float nearPlane, farPlane;
};

static void SetWorkgroupSize(unsigned w, unsigned h, unsigned d, unsigned groupSizeW, unsigned groupSizeH, unsigned groupSizeD, unsigned& dispatchW, unsigned& dispatchH, unsigned& dispatchD)
{
	//set up work group sizes
	unsigned gw = 0, gh = 0, gd = 0, count = 1;

	while (gw < w)
	{
		gw = groupSizeW * count;
		count++;
	}

	count = 1;

	while (gh < h)
	{
		gh = groupSizeH * count;
		count++;
	}

	count = 1;

	while (gd < d)
	{
		gd = groupSizeD * count;
		count++;
	}

	dispatchW = unsigned(float(gw) / groupSizeW);
	dispatchH = unsigned(float(gh) / groupSizeH);
	dispatchD = unsigned(float(gd) / groupSizeD);
}

static bool CheckMeshFormat(const Mesh& mesh) {
	for (size_t i = 0; i < mesh.GetNumStreams(); i++) {
		auto& elements = mesh.GetLayout()[0];
		if (elements.size() != 3) return false;
		if (elements[0].semantic != eVertexElementSemantic::POSITION) return false;
		if (elements[1].semantic != eVertexElementSemantic::NORMAL) return false;
		if (elements[2].semantic != eVertexElementSemantic::TEX_COORD) return false;
	}

	return true;
}


static void ConvertToSubmittable(
	Mesh* mesh,
	std::vector<const gxeng::VertexBuffer*>& vertexBuffers,
	std::vector<unsigned>& sizes,
	std::vector<unsigned>& strides
) {
	vertexBuffers.clear();
	sizes.clear();
	strides.clear();

	for (int streamID = 0; streamID < mesh->GetNumStreams(); streamID++) {
		vertexBuffers.push_back(&mesh->GetVertexBuffer(streamID));
		sizes.push_back((unsigned)vertexBuffers.back()->GetSize());
		strides.push_back((unsigned)mesh->GetVertexBufferStride(streamID));
	}

	assert(vertexBuffers.size() == sizes.size());
	assert(sizes.size() == strides.size());
}



Voxelization::Voxelization() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
	this->GetInput<2>().Set({});
	this->GetInput<3>().Set({});
	this->GetInput<4>().Set({});
}


void Voxelization::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void Voxelization::Reset() {
	m_voxelTexSRV = TextureView3D();
	m_voxelSecondaryTexSRV = TextureView3D();
	m_visualizationDSV = DepthStencilView2D();
	m_visualizationTexRTV = RenderTargetView2D();
	m_shadowCSMTexSrv = TextureView2D();
	GetInput(0)->Clear();
	GetInput(1)->Clear();
	GetInput(2)->Clear();
	GetInput(3)->Clear();
	GetInput(4)->Clear();
}


void Voxelization::Setup(SetupContext & context) {
	m_entities = this->GetInput<0>().Get();

	m_camera = this->GetInput<1>().Get();

	auto& target = this->GetInput<2>().Get();
	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_visualizationTexRTV = context.CreateRtv(target, target.GetFormat(), rtvDesc);
	

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	auto& depthStencil = this->GetInput<3>().Get();
	gxapi::DsvTexture2DArray dsvDesc;
	dsvDesc.activeArraySize = 1;
	dsvDesc.firstArrayElement = 0;
	dsvDesc.firstMipLevel = 0;
	m_visualizationDSV = context.CreateDsv(depthStencil, FormatAnyToDepthStencil(depthStencil.GetFormat()), dsvDesc);
	m_depthTexSRV = context.CreateSrv(depthStencil, FormatDepthToColor(depthStencil.GetFormat()), srvDesc);
	

	srvDesc.activeArraySize = 4;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	Texture2D shadowCSMTex = this->GetInput<4>().Get();
	m_shadowCSMTexSrv = context.CreateSrv(shadowCSMTex, FormatDepthToColor(shadowCSMTex.GetFormat()), srvDesc);
	

	srvDesc.activeArraySize = 1;

	Texture2D shadowCSMExtentsTex = this->GetInput<5>().Get();
	m_shadowCSMExtentsTexSrv = context.CreateSrv(shadowCSMExtentsTex, FormatDepthToColor(shadowCSMExtentsTex.GetFormat()), srvDesc);

	Texture2D velocityNormalTex = this->GetInput<6>().Get();
	m_velocityNormalTexSrv = context.CreateSrv(velocityNormalTex, velocityNormalTex.GetFormat(), srvDesc);

	Texture2D albedoRoughnessMetalnessTex = this->GetInput<7>().Get();
	m_albedoRoughnessMetalnessTexSrv = context.CreateSrv(albedoRoughnessMetalnessTex, albedoRoughnessMetalnessTex.GetFormat(), srvDesc);
	

	if (!m_binder.has_value()) {
		BindParameterDesc uniformsBindParamDesc;
		m_uniformsBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
		uniformsBindParamDesc.parameter = m_uniformsBindParam;
		uniformsBindParamDesc.constantSize = sizeof(Uniforms);
		uniformsBindParamDesc.relativeAccessFrequency = 0;
		uniformsBindParamDesc.relativeChangeFrequency = 0;
		uniformsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc;
		sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
		sampBindParamDesc.constantSize = 0;
		sampBindParamDesc.relativeAccessFrequency = 0;
		sampBindParamDesc.relativeChangeFrequency = 0;
		sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc1;
		sampBindParamDesc1.parameter = BindParameter(eBindParameterType::SAMPLER, 1);
		sampBindParamDesc1.constantSize = 0;
		sampBindParamDesc1.relativeAccessFrequency = 0;
		sampBindParamDesc1.relativeChangeFrequency = 0;
		sampBindParamDesc1.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc2;
		sampBindParamDesc2.parameter = BindParameter(eBindParameterType::SAMPLER, 2);
		sampBindParamDesc2.constantSize = 0;
		sampBindParamDesc2.relativeAccessFrequency = 0;
		sampBindParamDesc2.relativeChangeFrequency = 0;
		sampBindParamDesc2.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc voxelTexBindParamDesc;
		m_voxelTexBindParam = BindParameter(eBindParameterType::UNORDERED, 0);
		voxelTexBindParamDesc.parameter = m_voxelTexBindParam;
		voxelTexBindParamDesc.constantSize = 0;
		voxelTexBindParamDesc.relativeAccessFrequency = 0;
		voxelTexBindParamDesc.relativeChangeFrequency = 0;
		voxelTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc voxelLightTexBindParamDesc;
		m_voxelLightTexBindParam = BindParameter(eBindParameterType::UNORDERED, 1);
		voxelLightTexBindParamDesc.parameter = m_voxelLightTexBindParam;
		voxelLightTexBindParamDesc.constantSize = 0;
		voxelLightTexBindParamDesc.relativeAccessFrequency = 0;
		voxelLightTexBindParamDesc.relativeChangeFrequency = 0;
		voxelLightTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc voxelSecondaryTexBindParamDesc;
		m_voxelSecondaryTexBindParam = BindParameter(eBindParameterType::UNORDERED, 2);
		voxelSecondaryTexBindParamDesc.parameter = m_voxelSecondaryTexBindParam;
		voxelSecondaryTexBindParamDesc.constantSize = 0;
		voxelSecondaryTexBindParamDesc.relativeAccessFrequency = 0;
		voxelSecondaryTexBindParamDesc.relativeChangeFrequency = 0;
		voxelSecondaryTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc shadowCSMTexBindParamDesc;
		m_shadowCSMTexBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		shadowCSMTexBindParamDesc.parameter = m_shadowCSMTexBindParam;
		shadowCSMTexBindParamDesc.constantSize = 0;
		shadowCSMTexBindParamDesc.relativeAccessFrequency = 0;
		shadowCSMTexBindParamDesc.relativeChangeFrequency = 0;
		shadowCSMTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc shadowCSMExtentsTexBindParamDesc;
		m_shadowCSMExtentsTexBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		shadowCSMExtentsTexBindParamDesc.parameter = m_shadowCSMExtentsTexBindParam;
		shadowCSMExtentsTexBindParamDesc.constantSize = 0;
		shadowCSMExtentsTexBindParamDesc.relativeAccessFrequency = 0;
		shadowCSMExtentsTexBindParamDesc.relativeChangeFrequency = 0;
		shadowCSMExtentsTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc albedoTexBindParamDesc;
		m_albedoTexBindParam = BindParameter(eBindParameterType::TEXTURE, 2);
		albedoTexBindParamDesc.parameter = m_albedoTexBindParam;
		albedoTexBindParamDesc.constantSize = 0;
		albedoTexBindParamDesc.relativeAccessFrequency = 0;
		albedoTexBindParamDesc.relativeChangeFrequency = 0;
		albedoTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc voxelSecondaryTexReadBindParamDesc;
		m_voxelSecondaryTexReadBindParam = BindParameter(eBindParameterType::TEXTURE, 3);
		voxelSecondaryTexReadBindParamDesc.parameter = m_voxelSecondaryTexReadBindParam;
		voxelSecondaryTexReadBindParamDesc.constantSize = 0;
		voxelSecondaryTexReadBindParamDesc.relativeAccessFrequency = 0;
		voxelSecondaryTexReadBindParamDesc.relativeChangeFrequency = 0;
		voxelSecondaryTexReadBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc velocityNormalTexBindParamDesc;
		m_velocityNormalTexBindParam = BindParameter(eBindParameterType::TEXTURE, 4);
		velocityNormalTexBindParamDesc.parameter = m_velocityNormalTexBindParam;
		velocityNormalTexBindParamDesc.constantSize = 0;
		velocityNormalTexBindParamDesc.relativeAccessFrequency = 0;
		velocityNormalTexBindParamDesc.relativeChangeFrequency = 0;
		velocityNormalTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc albedoRoughnessMetalnessTexBindParamDesc;
		m_albedoRoughnessMetalnessTexBindParam = BindParameter(eBindParameterType::TEXTURE, 5);
		albedoRoughnessMetalnessTexBindParamDesc.parameter = m_velocityNormalTexBindParam;
		albedoRoughnessMetalnessTexBindParamDesc.constantSize = 0;
		albedoRoughnessMetalnessTexBindParamDesc.relativeAccessFrequency = 0;
		albedoRoughnessMetalnessTexBindParamDesc.relativeChangeFrequency = 0;
		albedoRoughnessMetalnessTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc1;
		samplerDesc1.shaderRegister = 1;
		samplerDesc1.filter = gxapi::eTextureFilterMode::MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc1.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc1.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc1.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc1.mipLevelBias = 0.f;
		samplerDesc1.registerSpace = 0;
		samplerDesc1.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc2;
		samplerDesc2.shaderRegister = 2;
		samplerDesc2.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samplerDesc2.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc2.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc2.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc2.mipLevelBias = 0.f;
		samplerDesc2.registerSpace = 0;
		samplerDesc2.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, sampBindParamDesc1, sampBindParamDesc2, voxelTexBindParamDesc, voxelSecondaryTexReadBindParamDesc, voxelSecondaryTexBindParamDesc, shadowCSMTexBindParamDesc, voxelLightTexBindParamDesc, shadowCSMExtentsTexBindParamDesc, albedoTexBindParamDesc, velocityNormalTexBindParamDesc, albedoRoughnessMetalnessTexBindParamDesc },{ samplerDesc, samplerDesc1, samplerDesc2 });
	}

	if (!m_shader.vs || !m_shader.gs || !m_shader.ps) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.gs = true;
		shaderParts.ps = true;

		m_shader = context.CreateShader("Voxelization", shaderParts, "");

		m_visualizerShader = context.CreateShader("VoxelVisualizer", shaderParts, "");

		shaderParts.gs = false;
		m_lightInjectionCSMShader = context.CreateShader("VoxelLightInjectionCSM", shaderParts, "");

		shaderParts.cs = true;
		shaderParts.vs = false;
		shaderParts.ps = false;
		shaderParts.gs = false;
		m_mipmapShader = context.CreateShader("VoxelMipmap", shaderParts, "");
	}

	if (m_PSO == nullptr) {
		InitRenderTarget(context);

		{
			std::vector<gxapi::InputElementDesc> inputElementDesc = {
				gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
				gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
				gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
			};

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_shader.vs;
			psoDesc.gs = m_shader.gs;
			psoDesc.ps = m_shader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			bool peti = false;
			if (!peti)
			{
				psoDesc.rasterization.conservativeRasterization = gxapi::eConservativeRasterizationMode::ON;
			}
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.blending.singleTarget.mask = {};
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.numRenderTargets = 0;

			m_PSO.reset(context.CreatePSO(psoDesc));
		}

		{ //light injection from a cascaded shadow map
			std::vector<gxapi::InputElementDesc> inputElementDesc2 = {
				gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
				gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
			};

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc2.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc2.size();
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_lightInjectionCSMShader.vs;
			psoDesc.ps = m_lightInjectionCSMShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.blending.singleTarget.mask = {};

			psoDesc.numRenderTargets = 0;

			m_lightInjectionCSMPSO.reset(context.CreatePSO(psoDesc));
		}

		{ //visualizer shader
			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = {};
			psoDesc.inputLayout.numElements = 0;
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_visualizerShader.vs;
			psoDesc.gs = m_visualizerShader.gs;
			psoDesc.ps = m_visualizerShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_CCW);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::POINT;
			psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
			psoDesc.depthStencilState.depthFunc = gxapi::eComparisonFunction::LESS;
			psoDesc.depthStencilFormat = m_visualizationDSV.GetDescription().format;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_visualizationTexRTV.GetResource().GetFormat();

			m_visualizerPSO.reset(context.CreatePSO(psoDesc));
		}

		{ //final gather shader
			std::vector<gxapi::InputElementDesc> inputElementDesc = {
				gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
				gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
			};

			ShaderParts shaderParts;
			shaderParts.vs = true;
			shaderParts.ps = true;

			m_finalGatherShader = context.CreateShader("VoxelFinalGather", shaderParts, "");

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_finalGatherShader.vs;
			psoDesc.ps = m_finalGatherShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
			psoDesc.depthStencilState = gxapi::DepthStencilState(false, false);
			psoDesc.depthStencilState.depthFunc = gxapi::eComparisonFunction::LESS;
			psoDesc.depthStencilFormat = m_visualizationDSV.GetDescription().format;

			gxapi::RenderTargetBlendState blending;
			blending.enableBlending = true;
			blending.alphaOperation = gxapi::eBlendOperation::ADD;
			blending.shaderAlphaFactor = gxapi::eBlendOperand::ONE;
			blending.targetAlphaFactor = gxapi::eBlendOperand::ONE;
			blending.colorOperation = gxapi::eBlendOperation::ADD;
			blending.shaderColorFactor = gxapi::eBlendOperand::ONE;
			blending.targetColorFactor = gxapi::eBlendOperand::ONE;
			blending.enableLogicOp = false;
			blending.mask = gxapi::eColorMask::ALL;

			psoDesc.blending.singleTarget = blending;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_visualizationTexRTV.GetResource().GetFormat();

			m_finalGatherPSO.reset(context.CreatePSO(psoDesc));
		}

		{ //mipmap gen shader
			gxapi::ComputePipelineStateDesc csoDesc;
			csoDesc.rootSignature = m_binder->GetRootSignature();
			csoDesc.cs = m_mipmapShader.cs;

			m_mipmapCSO.reset(context.CreatePSO(csoDesc));
		}
	}

	this->GetOutput<0>().Set(m_voxelTexUAV[0].GetResource());
	this->GetOutput<1>().Set(m_visualizationTexRTV.GetResource());
	this->GetOutput<2>().Set(m_visualizationDSV.GetResource());
}


void Voxelization::Execute(RenderContext & context) {
	if (!m_entities) {
		return;
	}

	bool peti = false;
	if (peti)
	{
		return;
	}

	Uniforms uniformsCBData;

	uniformsCBData.voxelDimension = voxelDimension;
	uniformsCBData.voxelCenter = voxelCenter;
	uniformsCBData.voxelSize = voxelSize;

	uniformsCBData.nearPlane = m_camera->GetNearPlane();
	uniformsCBData.farPlane = m_camera->GetFarPlane();
	Mat44 v = m_camera->GetViewMatrix();
	Mat44 p = m_camera->GetProjectionMatrix();
	Mat44 invV = v.Inverse();
	Mat44 invP = p.Inverse();
	uniformsCBData.wsCamPos = Vec4(m_camera->GetPosition(), 1.0);
	uniformsCBData.invView = invV;

	//far ndc corners
	Vec4 ndcCorners[] =
	{
		Vec4(-1.f, -1.f, 1.f, 1.f),
		Vec4(1.f, 1.f, 1.f, 1.f),
	};

	//convert to world space frustum corners
	ndcCorners[0] = ndcCorners[0] * invP;
	ndcCorners[1] = ndcCorners[1] * invP;
	ndcCorners[0] /= ndcCorners[0].w;
	ndcCorners[1] /= ndcCorners[1].w;

	uniformsCBData.farPlaneData0 = Vec4(ndcCorners[0].xyz, ndcCorners[1].x);
	uniformsCBData.farPlaneData1 = Vec4(ndcCorners[1].y, ndcCorners[1].z, 0.0f, 0.0f);

	uniformsCBData.viewProj = m_camera->GetViewMatrix() * m_camera->GetProjectionMatrix();

	auto& commandList = context.AsGraphics();

	gxapi::Rectangle rect{ 0, (int)m_voxelTexUAV[0].GetResource().GetHeight(), 0, (int)m_voxelTexUAV[0].GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder.value());
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	std::vector<const gxeng::VertexBuffer*> vertexBuffers;
	std::vector<unsigned> sizes;
	std::vector<unsigned> strides;

	commandList.SetResourceState(m_shadowCSMTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_shadowCSMExtentsTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_voxelLightTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_voxelTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_voxelSecondaryTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.BindGraphics(m_voxelTexBindParam, m_voxelTexUAV[0]);
	commandList.BindGraphics(m_voxelSecondaryTexBindParam, m_voxelSecondaryTexUAV[0]);
	commandList.BindGraphics(m_voxelLightTexBindParam, m_voxelLightTexUAV[0]);
	commandList.BindGraphics(m_shadowCSMTexBindParam, m_shadowCSMTexSrv);
	commandList.BindGraphics(m_shadowCSMExtentsTexBindParam, m_shadowCSMExtentsTexSrv);

	static bool sceneVoxelized = false;

	if (!sceneVoxelized)
	{
		{ // scene voxelization
			for (const MeshEntity* entity : *m_entities) {
				// Get entity parameters
				Mesh* mesh = entity->GetMesh();
				Material* material = entity->GetMaterial();
				auto position = entity->GetPosition();

				if (mesh->GetIndexBuffer().GetIndexCount() == 3600)
				{
					continue; //skip quadcopter for visualization purposes (obscures camera...)
				}

				// Draw mesh
				if (!CheckMeshFormat(*mesh)) {
					assert(false);
					continue;
				}

				ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);

				uniformsCBData.model = entity->GetTransform();

				commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

				for (size_t paramIdx = 0; paramIdx < material->GetParameterCount(); ++paramIdx) {
					const Material::Parameter& param = (*material)[paramIdx];
					if (param.GetType() == eMaterialShaderParamType::BITMAP_COLOR_2D ||
						param.GetType() == eMaterialShaderParamType::BITMAP_VALUE_2D)
					{
						commandList.SetResourceState(((Image*)param)->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
						commandList.BindGraphics(m_albedoTexBindParam, ((Image*)param)->GetSrv());
						break;
					}
				}

				for (auto& vb : vertexBuffers) {
					commandList.SetResourceState(*vb, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
				}

				commandList.SetResourceState(mesh->GetIndexBuffer(), gxapi::eResourceState::INDEX_BUFFER);
				commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
				commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
				commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
			}

			commandList.UAVBarrier(m_voxelTexUAV[0].GetResource());
			commandList.UAVBarrier(m_voxelSecondaryTexUAV[0].GetResource());
		}

		{ //scene mipmap gen
			int numMips = m_voxelTexSRV.GetResource().GetNumMiplevels();
			int currDim = voxelDimension / 2;
			for (int c = 1; c < numMips; ++c)
			{
				unsigned dispatchW, dispatchH, dispatchD;
				SetWorkgroupSize(currDim, currDim, currDim, 8, 8, 8, dispatchW, dispatchH, dispatchD);

				commandList.SetPipelineState(m_mipmapCSO.get());
				//NOTE: must set compute binder before bind* calls
				commandList.SetComputeBinder(&m_binder.value());

				uniformsCBData.inputMipLevel = c - 1;
				uniformsCBData.outputMipLevel = c;
				
				commandList.BindCompute(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

				//gen mipmap for primary voxel tex
				commandList.SetResourceState(m_voxelTexUAV[c].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, m_voxelTexSRV.GetResource().GetSubresourceIndex(c, 0));
				commandList.SetResourceState(m_voxelTexMipSRV[c-1].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE }, m_voxelTexSRV.GetResource().GetSubresourceIndex(c - 1, 0));

				commandList.BindCompute(m_voxelTexBindParam, m_voxelTexUAV[c]);
				commandList.BindCompute(m_shadowCSMTexBindParam, m_voxelTexMipSRV[c-1]);
				commandList.Dispatch(dispatchW, dispatchH, dispatchD);
				commandList.UAVBarrier(m_voxelTexUAV[c].GetResource());

				//gen mipmap for secondary voxel tex
				commandList.SetResourceState(m_voxelSecondaryTexUAV[c].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, m_voxelSecondaryTexSRV.GetResource().GetSubresourceIndex(c, 0));
				commandList.SetResourceState(m_voxelSecondaryTexMipSRV[c-1].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE }, m_voxelSecondaryTexSRV.GetResource().GetSubresourceIndex(c - 1, 0));

				commandList.BindCompute(m_voxelTexBindParam, m_voxelSecondaryTexUAV[c]);
				commandList.BindCompute(m_shadowCSMTexBindParam, m_voxelSecondaryTexMipSRV[c-1]);
				commandList.Dispatch(dispatchW, dispatchH, dispatchD);
				commandList.UAVBarrier(m_voxelSecondaryTexUAV[c].GetResource());

				currDim = currDim / 2;
			}
		}
		
		sceneVoxelized = true;
	}

	{ //light injection
		gxapi::Rectangle rect{ 0, (int)m_shadowCSMTexSrv.GetResource().GetHeight(), 0, (int)m_shadowCSMTexSrv.GetResource().GetWidth() };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetPipelineState(m_lightInjectionCSMPSO.get());
		commandList.SetGraphicsBinder(&m_binder.value());
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

		commandList.SetResourceState(m_voxelTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, m_voxelTexSRV.GetResource().GetSubresourceIndex(0, 0));
		commandList.SetResourceState(m_voxelSecondaryTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, m_voxelSecondaryTexSRV.GetResource().GetSubresourceIndex(0, 0));

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));
		commandList.BindGraphics(m_voxelTexBindParam, m_voxelTexUAV[0]);
		commandList.BindGraphics(m_voxelSecondaryTexBindParam, m_voxelSecondaryTexUAV[0]);
		commandList.BindGraphics(m_voxelLightTexBindParam, m_voxelLightTexUAV[0]);
		commandList.BindGraphics(m_shadowCSMTexBindParam, m_shadowCSMTexSrv);
		commandList.BindGraphics(m_shadowCSMExtentsTexBindParam, m_shadowCSMExtentsTexSrv);

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
		commandList.UAVBarrier(m_voxelLightTexUAV[0].GetResource()); 
	}

	{ //light voxel mipmap generation
		int numMips = m_voxelLightTexSRV.GetResource().GetNumMiplevels();
		int currDim = voxelDimension / 2;
		for (int c = 1; c < numMips; ++c)
		{
			unsigned dispatchW, dispatchH, dispatchD;
			SetWorkgroupSize(currDim, currDim, currDim, 8, 8, 8, dispatchW, dispatchH, dispatchD);

			commandList.SetPipelineState(m_mipmapCSO.get());
			commandList.SetComputeBinder(&m_binder.value());

			uniformsCBData.inputMipLevel = c - 1;
			uniformsCBData.outputMipLevel = c;

			commandList.BindCompute(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

			commandList.SetResourceState(m_voxelLightTexUAV[c].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, m_voxelLightTexSRV.GetResource().GetSubresourceIndex(c, 0));
			commandList.SetResourceState(m_voxelLightTexMipSRV[c-1].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE }, m_voxelLightTexSRV.GetResource().GetSubresourceIndex(c - 1, 0));

			commandList.BindCompute(m_voxelTexBindParam, m_voxelLightTexUAV[c]);
			commandList.BindCompute(m_shadowCSMTexBindParam, m_voxelLightTexMipSRV[c-1]);
			commandList.Dispatch(dispatchW, dispatchH, dispatchD);
			commandList.UAVBarrier(m_voxelLightTexUAV[c].GetResource());

			currDim = currDim / 2;
		}
	}

	/*{ //visualization
		gxapi::Rectangle rect{ 0, (int)m_visualizationTexRTV.GetResource().GetHeight(), 0, (int)m_visualizationTexRTV.GetResource().GetWidth() };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetResourceState(m_visualizationTexRTV.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_visualizationDSV.GetResource(), gxapi::eResourceState::DEPTH_WRITE);

		RenderTargetView2D* pRTV = &m_visualizationTexRTV;
		commandList.SetRenderTargets(1, &pRTV, &m_visualizationDSV);

		commandList.SetPipelineState(m_visualizerPSO.get());
		commandList.SetGraphicsBinder(&m_binder.value());
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::POINTLIST);

		commandList.BindGraphics(m_voxelTexBindParam, m_voxelTexUAV[0]);
		//commandList.BindGraphics(m_voxelTexBindParam, m_voxelLightTexUAV);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));
		commandList.DrawInstanced(voxelDimension * voxelDimension * voxelDimension); //draw points, expand in geometry shader
	}*/

	{ //final gather
		gxapi::Rectangle rect{ 0, (int)m_visualizationTexRTV.GetResource().GetHeight(), 0, (int)m_visualizationTexRTV.GetResource().GetWidth() };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetResourceState(m_visualizationTexRTV.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_visualizationDSV.GetResource(), gxapi::eResourceState::DEPTH_WRITE);

		RenderTargetView2D* pRTV = &m_visualizationTexRTV;
		commandList.SetRenderTargets(1, &pRTV, 0);

		commandList.SetPipelineState(m_finalGatherPSO.get());
		commandList.SetGraphicsBinder(&m_binder.value());

		commandList.SetResourceState(m_voxelLightTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_voxelTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_voxelSecondaryTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_depthTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_velocityNormalTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_albedoRoughnessMetalnessTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		commandList.BindGraphics(m_shadowCSMTexBindParam, m_voxelLightTexSRV);
		commandList.BindGraphics(m_shadowCSMExtentsTexBindParam, m_voxelTexSRV);
		commandList.BindGraphics(m_voxelSecondaryTexReadBindParam, m_voxelSecondaryTexSRV);
		commandList.BindGraphics(m_albedoTexBindParam, m_depthTexSRV);
		commandList.BindGraphics(m_velocityNormalTexBindParam, m_velocityNormalTexSrv);
		commandList.BindGraphics(m_albedoRoughnessMetalnessTexBindParam, m_albedoRoughnessMetalnessTexSrv);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}
}

void Voxelization::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatVoxel = eFormat::R8G8B8A8_UNORM;
		//auto formatVoxel = eFormat::R16G16B16A16_FLOAT;
		//auto formatVoxel = eFormat::R32_UINT;

		gxapi::UavTexture3D uavDesc;
		uavDesc.depthSize = voxelDimension;
		uavDesc.firstDepthLayer = 0;
		uavDesc.mipLevel = 0;

		gxapi::SrvTexture3D srvDesc;
		srvDesc.mipLevelClamping = 0;
		srvDesc.mostDetailedMip = 0;
		srvDesc.numMipLevels = -1;
		

		gxeng::Texture3DDesc texDesc;
		texDesc.width = (uint64_t)voxelDimension;
		texDesc.height = (uint64_t)voxelDimension;
		texDesc.depth = (uint64_t)voxelDimension;
		texDesc.format = formatVoxel;
		texDesc.mipLevels = 0;

		//TODO init to 0
		Texture3D voxel_tex = context.CreateTexture3D(texDesc, { true, false, false, true });
		voxel_tex.SetName("Voxelization voxel tex");	

		m_voxelTexUAV.resize(voxel_tex.GetNumMiplevels());
		m_voxelTexMipSRV.resize(voxel_tex.GetNumMiplevels());
		m_voxelLightTexUAV.resize(voxel_tex.GetNumMiplevels());
		m_voxelLightTexMipSRV.resize(voxel_tex.GetNumMiplevels());
		m_voxelSecondaryTexUAV.resize(voxel_tex.GetNumMiplevels());
		m_voxelSecondaryTexMipSRV.resize(voxel_tex.GetNumMiplevels());

		m_voxelTexSRV = context.CreateSrv(voxel_tex, formatVoxel, srvDesc);
		uavDesc.depthSize = voxelDimension;
		for (unsigned c = 0; c < voxel_tex.GetNumMiplevels(); ++c)
		{
			uavDesc.mipLevel = c;
			m_voxelTexUAV[c] = context.CreateUav(voxel_tex, formatVoxel, uavDesc);
			srvDesc.numMipLevels = 1;
			srvDesc.mostDetailedMip = c;
			m_voxelTexMipSRV[c] = context.CreateSrv(voxel_tex, formatVoxel, srvDesc);
			uavDesc.depthSize = uavDesc.depthSize / 2;
		}

		Texture3D secondaryVoxelTex = context.CreateTexture3D(texDesc, { true, false, false, true });
		secondaryVoxelTex.SetName("Voxelization voxel secondary tex");

		srvDesc.mostDetailedMip = 0;
		srvDesc.numMipLevels = -1;
		m_voxelSecondaryTexSRV = context.CreateSrv(secondaryVoxelTex, formatVoxel, srvDesc);
		uavDesc.depthSize = voxelDimension;
		for (unsigned c = 0; c < secondaryVoxelTex.GetNumMiplevels(); ++c)
		{
			uavDesc.mipLevel = c;
			m_voxelSecondaryTexUAV[c] = context.CreateUav(secondaryVoxelTex, formatVoxel, uavDesc);
			srvDesc.numMipLevels = 1;
			srvDesc.mostDetailedMip = c;
			m_voxelSecondaryTexMipSRV[c] = context.CreateSrv(secondaryVoxelTex, formatVoxel, srvDesc);
			uavDesc.depthSize = uavDesc.depthSize / 2;
		}
		

		Texture3D voxelLightTex = context.CreateTexture3D(texDesc, { true, false, false, true });
		voxelLightTex.SetName("Voxelization voxel tex");

		srvDesc.mostDetailedMip = 0;
		srvDesc.numMipLevels = -1;
		m_voxelLightTexSRV = context.CreateSrv(voxelLightTex, formatVoxel, srvDesc);
		uavDesc.depthSize = voxelDimension;
		for (unsigned c = 0; c < voxel_tex.GetNumMiplevels(); ++c)
		{
			uavDesc.mipLevel = c;
			m_voxelLightTexUAV[c] = context.CreateUav(voxelLightTex, formatVoxel, uavDesc);
			srvDesc.numMipLevels = 1;
			srvDesc.mostDetailedMip = c;
			m_voxelLightTexMipSRV[c] = context.CreateSrv(voxelLightTex, formatVoxel, srvDesc);
			uavDesc.depthSize = uavDesc.depthSize / 2;
		}
	}
}


} // namespace inl::gxeng::nodes
