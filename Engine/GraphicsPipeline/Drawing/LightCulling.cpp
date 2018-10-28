#include "LightCulling.hpp"

#include "../Debug/DebugDrawManager.hpp"
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>



namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(LightCulling)


struct LightData {
	Vec4_Packed vsPosition;
	float attenuationEnd;
	Vec3_Packed dummy;
};

struct Uniforms {
	LightData ld[10];
	Mat44_Packed p;
	Vec4_Packed farPlane0, farPlane1;
	float camNear, camFar;
	uint32_t numLights, numWorkgroupsX, numWorkgroupsY;
	Vec3_Packed dummy;
};

static void SetWorkgroupSize(unsigned w, unsigned h, unsigned groupSizeW, unsigned groupSizeH, unsigned& dispatchW, unsigned& dispatchH) {
	//set up work group sizes
	unsigned gw = 0, gh = 0, count = 1;

	while (gw < w) {
		gw = groupSizeW * count;
		count++;
	}

	count = 1;

	while (gh < h) {
		gh = groupSizeH * count;
		count++;
	}

	dispatchW = unsigned(float(gw) / groupSizeW);
	dispatchH = unsigned(float(gh) / groupSizeH);
}


LightCulling::LightCulling() {
	this->GetInput<0>().Set({});
}


void LightCulling::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void LightCulling::Reset() {
	m_depthTexSrv = TextureView2D();
	m_camera = nullptr;
	//m_suns = nullptr;

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	//GetInput<2>().Clear();
}

const std::string& LightCulling::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"depthTex",
		"camera"
	};
	return names[index];
}

const std::string& LightCulling::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"lightCullData"
	};
	return names[index];
}

void LightCulling::Setup(SetupContext& context) {
	Texture2D depthTex = this->GetInput<0>().Get();

	if (depthTex.GetWidth() != m_width || depthTex.GetHeight() != m_height) {
		m_width = depthTex.GetWidth();
		m_height = depthTex.GetHeight();
		InitRenderTarget(context);
	}

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;
	m_depthTexSrv = context.CreateSrv(depthTex, FormatDepthToColor(depthTex.GetFormat()), srvDesc);


	m_camera = this->GetInput<1>().Get();
	//m_suns = this->GetInput<2>().Get();

	if (!m_binder) {
		BindParameterDesc uniformsBindParamDesc;
		m_uniformsBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
		uniformsBindParamDesc.parameter = m_uniformsBindParam;
		uniformsBindParamDesc.constantSize = sizeof(Uniforms); //TODO verify?
		uniformsBindParamDesc.relativeAccessFrequency = 0;
		uniformsBindParamDesc.relativeChangeFrequency = 0;
		uniformsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc;
		sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
		sampBindParamDesc.constantSize = 0;
		sampBindParamDesc.relativeAccessFrequency = 0;
		sampBindParamDesc.relativeChangeFrequency = 0;
		sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc reductionBindParamDesc;
		m_inputBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		reductionBindParamDesc.parameter = m_inputBindParam;
		reductionBindParamDesc.constantSize = 0;
		reductionBindParamDesc.relativeAccessFrequency = 0;
		reductionBindParamDesc.relativeChangeFrequency = 0;
		reductionBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc outputBindParamDesc0;
		m_outputBindParam = BindParameter(eBindParameterType::UNORDERED, 0);
		outputBindParamDesc0.parameter = m_outputBindParam;
		outputBindParamDesc0.constantSize = 0;
		outputBindParamDesc0.relativeAccessFrequency = 0;
		outputBindParamDesc0.relativeChangeFrequency = 0;
		outputBindParamDesc0.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, reductionBindParamDesc, outputBindParamDesc0 }, { samplerDesc });
	}

	if (!m_CSO) {
		ShaderParts shaderParts;
		shaderParts.cs = true;

		m_shader = context.CreateShader("LightCulling", shaderParts, "");

		gxapi::ComputePipelineStateDesc csoDesc;
		csoDesc.rootSignature = m_binder.GetRootSignature();
		csoDesc.cs = m_shader.cs;

		m_CSO.reset(context.CreatePSO(csoDesc));
	}

	this->GetOutput<0>().Set(m_lightCullDataUAV.GetResource());
}


void LightCulling::Execute(RenderContext& context) {
	ComputeCommandList& commandList = context.AsCompute();

	Uniforms uniformsCBData;

	uniformsCBData.camNear = m_camera->GetNearPlane();
	uniformsCBData.camFar = m_camera->GetFarPlane();
	uniformsCBData.numLights = 1;
	uniformsCBData.p = m_camera->GetProjectionMatrix();

	Mat44 invVP = (m_camera->GetViewMatrix() * m_camera->GetProjectionMatrix()).Inverse();

	//far ndc corners
	Vec4 ndcCorners[] =
		{
			Vec4(-1.f, -1.f, 1.f, 1.f),
			Vec4(1.f, 1.f, 1.f, 1.f),
		};

	//convert to world space frustum corners
	ndcCorners[0] = ndcCorners[0] * invVP;
	ndcCorners[1] = ndcCorners[1] * invVP;
	ndcCorners[0] /= ndcCorners[0].w;
	ndcCorners[1] /= ndcCorners[1].w;

	uniformsCBData.farPlane0 = Vec4(ndcCorners[0].xyz, ndcCorners[1].x);
	uniformsCBData.farPlane1 = Vec4(ndcCorners[1].y, ndcCorners[1].z, 0.0f, 0.0f);

	uniformsCBData.ld[0].vsPosition = Vec4(Vec3(0, 0, 1), 1.0f) * m_camera->GetViewMatrix();
	//uniformsCBData.ld[0].vs_position = Vec4(m_camera->GetPosition() + m_camera->GetLookDirection() * 5.f, 1.0f) * m_camera->GetViewMatrix();
	uniformsCBData.ld[0].attenuationEnd = 5.0f;

	//DebugDrawManager::GetInstance().AddSphere(Vec3(0, 0, 1), 5.0f, 0);

	uint32_t dispatchW, dispatchH;
	SetWorkgroupSize((unsigned)m_width, (unsigned)m_height, 16, 16, dispatchW, dispatchH);

	uniformsCBData.numWorkgroupsX = dispatchW;
	uniformsCBData.numWorkgroupsY = dispatchH;

	//create single-frame only cb
	gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Light culling volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));


	commandList.SetResourceState(m_lightCullDataUAV.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	commandList.SetPipelineState(m_CSO.get());
	commandList.SetComputeBinder(&m_binder);
	commandList.BindCompute(m_inputBindParam, m_depthTexSrv);
	commandList.BindCompute(m_outputBindParam, m_lightCullDataUAV);
	commandList.BindCompute(m_uniformsBindParam, cbv);
	commandList.Dispatch(dispatchW, dispatchH, 1);
	commandList.UAVBarrier(m_lightCullDataUAV.GetResource());
}


void LightCulling::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatLightCullData = eFormat::R32_UINT;

		gxapi::UavTexture2DArray uavDesc;
		uavDesc.activeArraySize = 1;
		uavDesc.firstArrayElement = 0;
		uavDesc.mipLevel = 0;
		uavDesc.planeIndex = 0;

		gxapi::SrvTexture2DArray srvDesc;
		srvDesc.activeArraySize = 1;
		srvDesc.firstArrayElement = 0;
		srvDesc.numMipLevels = -1;
		srvDesc.mipLevelClamping = 0;
		srvDesc.mostDetailedMip = 0;
		srvDesc.planeIndex = 0;

		uint32_t dispatchW, dispatchH;
		SetWorkgroupSize((unsigned)m_width, (unsigned)m_height, 16, 16, dispatchW, dispatchH);

		//TODO 1D tex
		Texture2D lightCullDataTex = context.CreateTexture2D({ dispatchW * dispatchH, 1024, formatLightCullData }, { true, true, false, true });
		lightCullDataTex.SetName("Light culling light cull data tex");
		m_lightCullDataUAV = context.CreateUav(lightCullDataTex, formatLightCullData, uavDesc);
	}
}


} // namespace inl::gxeng::nodes
