#include "VoxelLighting.hpp"

#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>



namespace inl::gxeng::nodes {

INL_REGISTER_GRAPHICS_NODE(VoxelLighting)


const int voxelDimension = 256; //units
const float voxelSize = 0.16f; //meters
const Vec3 voxelOrigin = Vec3(voxelDimension * voxelSize * -0.5);
const Vec3 voxelCenter = Vec3(0.0f);

struct Uniforms {
	Mat44_Packed model, viewProj, invView;
	Vec3_Packed voxelCenter;
	float voxelSize;
	Vec4_Packed farPlaneData0, farPlaneData1, wsCamPos;
	int voxelDimension;
	int inputMipLevel;
	int outputMipLevel;
	int dummy;
	float nearPlane, farPlane;
};

static void SetWorkgroupSize(unsigned w, unsigned h, unsigned d, unsigned groupSizeW, unsigned groupSizeH, unsigned groupSizeD, unsigned& dispatchW, unsigned& dispatchH, unsigned& dispatchD) {
	//set up work group sizes
	unsigned gw = 0, gh = 0, gd = 0, count = 1;

	while (gw < w) {
		gw = groupSizeW * count;
		count++;
	}

	count = 1;

	while (gh < h) {
		gh = groupSizeH * count;
		count++;
	}

	count = 1;

	while (gd < d) {
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
		if (elements.size() != 3)
			return false;
		if (elements[0].semantic != eVertexElementSemantic::POSITION)
			return false;
		if (elements[1].semantic != eVertexElementSemantic::NORMAL)
			return false;
		if (elements[2].semantic != eVertexElementSemantic::TEX_COORD)
			return false;
	}

	return true;
}


static void ConvertToSubmittable(
	Mesh* mesh,
	std::vector<const gxeng::VertexBuffer*>& vertexBuffers,
	std::vector<unsigned>& sizes,
	std::vector<unsigned>& strides) {
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



VoxelLighting::VoxelLighting() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
	this->GetInput<2>().Set({});
	this->GetInput<3>().Set({});
	this->GetInput<4>().Set({});
}


void VoxelLighting::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void VoxelLighting::Reset() {
	m_voxelColorTexSRV = TextureView3D();
	m_voxelAlphaNormalTexSRV = TextureView3D();
	m_visualizationDSV = DepthStencilView2D();
	m_visualizationTexRTV = RenderTargetView2D();
	m_shadowCSMTexSrv = TextureView2D();
	GetInput(0)->Clear();
	GetInput(1)->Clear();
	GetInput(2)->Clear();
	GetInput(3)->Clear();
	GetInput(4)->Clear();
}

const std::string& VoxelLighting::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"camera",
		"voxelColorTex",
		"voxelAlphaNormalTex",
		"colorRTV",
		"depthStencil",
		"shadowCSMTex",
		"shadowCSMExtentsTex",
		"velocityNormalTex",
		"albedoRoughnessMetalnessTex",
		"screenSpaceAmbientOcclusion"
	};
	return names[index];
}

const std::string& VoxelLighting::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"colorOutput",
		"depthStencilOutput"
	};
	return names[index];
}

void VoxelLighting::Setup(SetupContext& context) {
	m_camera = this->GetInput<0>().Get();

	auto& voxelColorTex = this->GetInput<1>().Get();
	auto& voxelAlphaNormalTex = this->GetInput<2>().Get();
	gxapi::SrvTexture3D srvDesc3D;
	srvDesc3D.mipLevelClamping = 0;
	srvDesc3D.mostDetailedMip = 0;
	srvDesc3D.numMipLevels = 1;

	m_voxelColorTexSRV = context.CreateSrv(voxelColorTex, voxelColorTex.GetFormat(), srvDesc3D);

	m_voxelAlphaNormalTexSRV = context.CreateSrv(voxelColorTex, voxelColorTex.GetFormat(), srvDesc3D);

	auto& target = this->GetInput<3>().Get();
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

	auto& depthStencil = this->GetInput<4>().Get();
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

	Texture2D shadowCSMTex = this->GetInput<5>().Get();
	m_shadowCSMTexSrv = context.CreateSrv(shadowCSMTex, FormatDepthToColor(shadowCSMTex.GetFormat()), srvDesc);


	srvDesc.activeArraySize = 1;

	Texture2D shadowCSMExtentsTex = this->GetInput<6>().Get();
	m_shadowCSMExtentsTexSrv = context.CreateSrv(shadowCSMExtentsTex, FormatDepthToColor(shadowCSMExtentsTex.GetFormat()), srvDesc);

	Texture2D velocityNormalTex = this->GetInput<7>().Get();
	m_velocityNormalTexSrv = context.CreateSrv(velocityNormalTex, velocityNormalTex.GetFormat(), srvDesc);

	Texture2D albedoRoughnessMetalnessTex = this->GetInput<8>().Get();
	m_albedoRoughnessMetalnessTexSrv = context.CreateSrv(albedoRoughnessMetalnessTex, albedoRoughnessMetalnessTex.GetFormat(), srvDesc);

	Texture2D screenSpaceAmbientOcclusionTex = this->GetInput<9>().Get();
	m_screenSpaceAmbientOcclusionTexSrv = context.CreateSrv(screenSpaceAmbientOcclusionTex, screenSpaceAmbientOcclusionTex.GetFormat(), srvDesc);


	if (!m_binder) {
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
		m_voxelColorTexBindParam = BindParameter(eBindParameterType::UNORDERED, 0);
		voxelTexBindParamDesc.parameter = m_voxelColorTexBindParam;
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

		BindParameterDesc tex0BindParamDesc;
		m_tex0BindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		tex0BindParamDesc.parameter = m_tex0BindParam;
		tex0BindParamDesc.constantSize = 0;
		tex0BindParamDesc.relativeAccessFrequency = 0;
		tex0BindParamDesc.relativeChangeFrequency = 0;
		tex0BindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc tex1BindParamDesc;
		m_tex1BindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		tex1BindParamDesc.parameter = m_tex1BindParam;
		tex1BindParamDesc.constantSize = 0;
		tex1BindParamDesc.relativeAccessFrequency = 0;
		tex1BindParamDesc.relativeChangeFrequency = 0;
		tex1BindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc tex2BindParamDesc;
		m_tex2BindParam = BindParameter(eBindParameterType::TEXTURE, 2);
		tex2BindParamDesc.parameter = m_tex2BindParam;
		tex2BindParamDesc.constantSize = 0;
		tex2BindParamDesc.relativeAccessFrequency = 0;
		tex2BindParamDesc.relativeChangeFrequency = 0;
		tex2BindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc tex3BindParamDesc;
		m_tex3BindParam = BindParameter(eBindParameterType::TEXTURE, 3);
		tex3BindParamDesc.parameter = m_tex3BindParam;
		tex3BindParamDesc.constantSize = 0;
		tex3BindParamDesc.relativeAccessFrequency = 0;
		tex3BindParamDesc.relativeChangeFrequency = 0;
		tex3BindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc tex4BindParamDesc;
		m_tex4BindParam = BindParameter(eBindParameterType::TEXTURE, 4);
		tex4BindParamDesc.parameter = m_tex4BindParam;
		tex4BindParamDesc.constantSize = 0;
		tex4BindParamDesc.relativeAccessFrequency = 0;
		tex4BindParamDesc.relativeChangeFrequency = 0;
		tex4BindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc tex5BindParamDesc;
		m_tex5BindParam = BindParameter(eBindParameterType::TEXTURE, 5);
		tex5BindParamDesc.parameter = m_tex5BindParam;
		tex5BindParamDesc.constantSize = 0;
		tex5BindParamDesc.relativeAccessFrequency = 0;
		tex5BindParamDesc.relativeChangeFrequency = 0;
		tex5BindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc tex6BindParamDesc;
		m_tex6BindParam = BindParameter(eBindParameterType::TEXTURE, 6);
		tex6BindParamDesc.parameter = m_tex6BindParam;
		tex6BindParamDesc.constantSize = 0;
		tex6BindParamDesc.relativeAccessFrequency = 0;
		tex6BindParamDesc.relativeChangeFrequency = 0;
		tex6BindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

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

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, sampBindParamDesc1, sampBindParamDesc2, voxelTexBindParamDesc, voxelLightTexBindParamDesc, tex0BindParamDesc, tex1BindParamDesc, tex2BindParamDesc, tex3BindParamDesc, tex4BindParamDesc, tex5BindParamDesc, tex6BindParamDesc }, { samplerDesc, samplerDesc1, samplerDesc2 });
	}

	if (!m_lightInjectionCSMShader.vs || !m_lightInjectionCSMShader.gs || !m_lightInjectionCSMShader.ps) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.gs = true;
		shaderParts.ps = true;

		m_visualizerShader = context.CreateShader("VoxelVisualizer", shaderParts, "");

		shaderParts.gs = false;
		m_lightInjectionCSMShader = context.CreateShader("VoxelLightInjectionCSM", shaderParts, "");

		shaderParts.cs = true;
		shaderParts.vs = false;
		shaderParts.ps = false;
		shaderParts.gs = false;
		m_mipmapShader = context.CreateShader("VoxelMipmap", shaderParts, "");

		shaderParts.cs = false;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_finalGatherShader = context.CreateShader("VoxelFinalGather", shaderParts, "");
	}

	if (m_lightInjectionCSMPSO == nullptr) {
		InitRenderTarget(context);

		{ //light injection from a cascaded shadow map
			std::vector<gxapi::InputElementDesc> inputElementDesc2 = {
				gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
				gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
			};

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc2.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc2.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
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
			psoDesc.rootSignature = m_binder.GetRootSignature();
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

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
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
			csoDesc.rootSignature = m_binder.GetRootSignature();
			csoDesc.cs = m_mipmapShader.cs;

			m_mipmapCSO.reset(context.CreatePSO(csoDesc));
		}
	}

	this->GetOutput<0>().Set(m_visualizationTexRTV.GetResource());
	this->GetOutput<1>().Set(m_visualizationDSV.GetResource());
}


void VoxelLighting::Execute(RenderContext& context) {
	bool peti = false;
	if (peti) {
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

	commandList.SetGraphicsBinder(&m_binder);
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	std::vector<const gxeng::VertexBuffer*> vertexBuffers;
	std::vector<unsigned> sizes;
	std::vector<unsigned> strides;

	commandList.SetResourceState(m_shadowCSMTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_shadowCSMExtentsTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_voxelLightTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_voxelColorTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

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
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));
		commandList.BindGraphics(m_voxelLightTexBindParam, m_voxelLightTexUAV[0]);
		commandList.BindGraphics(m_tex0BindParam, m_voxelColorTexSRV);
		commandList.BindGraphics(m_tex1BindParam, m_shadowCSMTexSrv);
		commandList.BindGraphics(m_tex2BindParam, m_shadowCSMExtentsTexSrv);

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
		commandList.UAVBarrier(m_voxelLightTexUAV[0].GetResource());
	}

	{ //light voxel mipmap generation
		int numMips = m_voxelLightTexSRV.GetResource().GetNumMiplevels();
		int currDim = voxelDimension / 2;
		for (int c = 1; c < numMips; ++c) {
			unsigned dispatchW, dispatchH, dispatchD;
			SetWorkgroupSize(currDim, currDim, currDim, 8, 8, 8, dispatchW, dispatchH, dispatchD);

			commandList.SetPipelineState(m_mipmapCSO.get());
			commandList.SetComputeBinder(&m_binder);

			uniformsCBData.inputMipLevel = c - 1;
			uniformsCBData.outputMipLevel = c;

			commandList.BindCompute(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

			commandList.SetResourceState(m_voxelLightTexUAV[c].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, m_voxelLightTexSRV.GetResource().GetSubresourceIndex(c, 0));
			commandList.SetResourceState(m_voxelLightTexMipSRV[c - 1].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE }, m_voxelLightTexSRV.GetResource().GetSubresourceIndex(c - 1, 0));

			commandList.BindCompute(m_voxelColorTexBindParam, m_voxelLightTexUAV[c]);
			commandList.BindCompute(m_tex0BindParam, m_voxelLightTexMipSRV[c - 1]);
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
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::POINTLIST);

		commandList.BindGraphics(m_tex0BindParam, m_voxelTexSRV);

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
		commandList.SetGraphicsBinder(&m_binder);

		commandList.SetResourceState(m_voxelLightTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_voxelColorTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_voxelAlphaNormalTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_depthTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_velocityNormalTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_albedoRoughnessMetalnessTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_screenSpaceAmbientOcclusionTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		commandList.BindGraphics(m_tex0BindParam, m_voxelLightTexSRV);
		commandList.BindGraphics(m_tex1BindParam, m_voxelColorTexSRV);
		commandList.BindGraphics(m_tex2BindParam, m_depthTexSRV);
		commandList.BindGraphics(m_tex3BindParam, m_voxelAlphaNormalTexSRV);
		commandList.BindGraphics(m_tex4BindParam, m_velocityNormalTexSrv);
		commandList.BindGraphics(m_tex5BindParam, m_albedoRoughnessMetalnessTexSrv);
		commandList.BindGraphics(m_tex6BindParam, m_screenSpaceAmbientOcclusionTexSrv);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}
}

void VoxelLighting::InitRenderTarget(SetupContext& context) {
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

		m_voxelLightTexUAV.resize(m_voxelColorTexSRV.GetResource().GetNumMiplevels());
		m_voxelLightTexMipSRV.resize(m_voxelColorTexSRV.GetResource().GetNumMiplevels());

		Texture3D voxelLightTex = context.CreateTexture3D(texDesc, { true, false, false, true });
		voxelLightTex.SetName("VoxelLighting voxel tex");

		srvDesc.mostDetailedMip = 0;
		srvDesc.numMipLevels = -1;
		m_voxelLightTexSRV = context.CreateSrv(voxelLightTex, formatVoxel, srvDesc);
		uavDesc.depthSize = voxelDimension;
		for (unsigned c = 0; c < m_voxelColorTexSRV.GetResource().GetNumMiplevels(); ++c) {
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
