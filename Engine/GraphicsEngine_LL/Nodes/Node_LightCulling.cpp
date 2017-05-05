#include "Node_LightCulling.hpp"

#include "NodeUtility.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../PerspectiveCamera.hpp"
#include "../GraphicsCommandList.hpp"
#include "../EntityCollection.hpp"

#include <array>

namespace inl::gxeng::nodes {

struct light_data
{
	mathfu::VectorPacked<float, 4> vs_position;
	float attenuation_end;
	mathfu::VectorPacked<float, 2> dummy;
};

struct Uniforms
{
	light_data ld[10];
	mathfu::VectorPacked<float, 4> p[4];
	mathfu::VectorPacked<float, 4> far_plane0, far_plane1;
	float cam_near, cam_far;
	int num_lights;
	float dummy;
};

static void setWorkgroupSize(unsigned w, unsigned h, unsigned groupSizeW, unsigned groupSizeH, unsigned& dispatchW, unsigned& dispatchH)
{
	//set up work group sizes
	unsigned gw = 0, gh = 0, count = 1;

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

	dispatchW = unsigned(float(gw) / groupSizeW);
	dispatchH = unsigned(float(gh) / groupSizeH);
}


LightCulling::LightCulling() {
	this->GetInput<0>().Set({});
}


void LightCulling::Initialize(EngineContext & context) {
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

	if (!m_binder.has_value()) {
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
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, reductionBindParamDesc, outputBindParamDesc0 },{ samplerDesc });
	}

	if (!m_CSO) {
		ShaderParts shaderParts;
		shaderParts.cs = true;

		auto shader = context.CreateShader("LightCulling", shaderParts, "");

		gxapi::ComputePipelineStateDesc csoDesc;
		csoDesc.rootSignature = m_binder->GetRootSignature();
		csoDesc.cs = shader.cs;

		m_CSO.reset(context.CreatePSO(csoDesc));
	}

	this->GetOutput<0>().Set(m_lightCullDataUAV.GetResource());
}


void LightCulling::Execute(RenderContext& context) {
	ComputeCommandList& commandList = context.AsCompute();

	Uniforms uniformsCBData;

	uniformsCBData.cam_near = -m_camera->GetNearPlane();
	uniformsCBData.cam_far = -m_camera->GetFarPlane();
	uniformsCBData.num_lights = 1;
	m_camera->GetProjectionMatrixRH().Pack(uniformsCBData.p);

	mathfu::Matrix4x4f invVP = (m_camera->GetProjectionMatrixRH() * m_camera->GetViewMatrixRH()).Inverse();

	mathfu::Vector4f ndcCorners[] = 
	{
		mathfu::Vector4f(-1, -1, -1, 1),
		mathfu::Vector4f(1, 1, -1, 1),
	};

	//convert to world space frustum corners
	ndcCorners[0] = invVP * ndcCorners[0];
	ndcCorners[1] = invVP * ndcCorners[1];
	ndcCorners[0] /= ndcCorners[0].w();
	ndcCorners[1] /= ndcCorners[1].w();

	uniformsCBData.far_plane0 = mathfu::Vector4f(ndcCorners[0].xyz(), ndcCorners[1].x());
	uniformsCBData.far_plane1 = mathfu::Vector4f(ndcCorners[1].y(), ndcCorners[1].z(), 0, 0);

	uniformsCBData.ld[0].vs_position = m_camera->GetViewMatrixRH() * mathfu::Vector4f(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1.0f);
	uniformsCBData.ld[0].attenuation_end = 5.0f;

	//create single-frame only cb
	gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));

	commandList.SetResourceState(m_lightCullDataUAV.GetResource(), 0, gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_depthTexSrv.GetResource(), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);

	commandList.SetPipelineState(m_CSO.get());
	commandList.SetComputeBinder(&m_binder.value());
	commandList.BindCompute(m_inputBindParam, m_depthTexSrv);
	commandList.BindCompute(m_outputBindParam, m_lightCullDataUAV);
	commandList.BindCompute(m_uniformsBindParam, cbv);
	uint32_t dispatchW, dispatchH;
	setWorkgroupSize(m_width, m_height, 16, 16, dispatchW, dispatchH);
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
		setWorkgroupSize(m_width, m_height, 16, 16, dispatchW, dispatchH);

		//TODO 1D tex
		Texture2D lightCullDataTex = context.CreateRWTexture2D(dispatchW * dispatchH, 1024, formatLightCullData, 1);
		m_lightCullDataUAV = context.CreateUav(lightCullDataTex, formatLightCullData, uavDesc);
	}
}


} // namespace inl::gxeng::nodes
