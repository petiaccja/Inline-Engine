#include "VolumetricLighting.hpp"

#include "../Debug/DebugDrawManager.hpp"
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>

namespace inl::gxeng::nodes {

INL_REGISTER_GRAPHICS_NODE(VolumetricLighting)

struct SdfData {
	Vec4_Packed vsPosition;
	float radius;
	Vec3_Packed dummy;
}; //size: 32

struct LightData {
	Vec4_Packed diffuseLightColor;
	Vec4_Packed vsPosition;
	float attenuationEnd;
	Vec3_Packed dummy;
}; //size: 48

struct Uniforms {
	SdfData sd[10]; //320
	LightData ld[10]; //480
	Mat44_Packed v, p; //64
	Mat44_Packed invVP, oldVP; //128
	float camNear, camFar, dummy1, dummy2; //16
	uint32_t numSdfs, numWorkgroupsX, numWorkgroupsY;
	float haltonFactor; //16
	Vec4_Packed sunDirection; //16
	Vec4_Packed sunColor; //16
	Vec4_Packed camPos; //16
}; //1072

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

static float getHalton(uint32_t i, uint32_t b) {
	float f = 1.0;
	float r = 0.0;

	while (i > 0u) {
		f = f / float(b);
		r = r + f * float(i % b);
		i = i / b;
	}

	return r;
}

VolumetricLighting::VolumetricLighting() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
}


void VolumetricLighting::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void VolumetricLighting::Reset() {
	m_depthTexSrv = TextureView2D();
	m_sdfCullDataUAV = RWTextureView2D();
	m_colorTexSRV = TextureView2D();
	for (int c = 0; c < 2; ++c) {
		m_volDstTexUAV[c] = RWTextureView2D();
	}
	m_dstTexUAV = RWTextureView2D();
	m_camera = nullptr;

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
}

const std::string& VolumetricLighting::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"depthTex",
		"colorTex",
		"lightCullTex",
		"camera",
		"csmTex",
		"shadowMxTex",
		"csmSplitsTex"
	};
	return names[index];
}

const std::string& VolumetricLighting::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"sdfCullData"
	};
	return names[index];
}

void VolumetricLighting::Setup(SetupContext& context) {
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	Texture2D depthTex = this->GetInput<0>().Get();
	m_depthTexSrv = context.CreateSrv(depthTex, FormatDepthToColor(depthTex.GetFormat()), srvDesc);


	gxapi::UavTexture2DArray uavDesc;
	uavDesc.activeArraySize = 1;
	uavDesc.firstArrayElement = 0;
	uavDesc.mipLevel = 0;
	uavDesc.planeIndex = 0;

	Texture2D colorTex = this->GetInput<1>().Get();
	m_colorTexSRV = context.CreateSrv(colorTex, colorTex.GetFormat(), srvDesc);


	Texture2D lightCullTex = this->GetInput<2>().Get();
	m_lightCullDataSRV = context.CreateSrv(lightCullTex, lightCullTex.GetFormat(), srvDesc);


	m_camera = this->GetInput<3>().Get();

	srvDesc.activeArraySize = 4;

	Texture2D csmTex = this->GetInput<4>().Get();
	m_csmTexSRV = context.CreateSrv(csmTex, FormatDepthToColor(csmTex.GetFormat()), srvDesc);


	srvDesc.activeArraySize = 1;

	Texture2D shadowMXTex = this->GetInput<5>().Get();
	m_shadowMXTexSRV = context.CreateSrv(shadowMXTex, shadowMXTex.GetFormat(), srvDesc);


	Texture2D csmSplitsTex = this->GetInput<6>().Get();
	m_csmSplitsTexSRV = context.CreateSrv(csmSplitsTex, csmSplitsTex.GetFormat(), srvDesc);


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

		BindParameterDesc depthTexBindParamDesc;
		m_depthBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		depthTexBindParamDesc.parameter = m_depthBindParam;
		depthTexBindParamDesc.constantSize = 0;
		depthTexBindParamDesc.relativeAccessFrequency = 0;
		depthTexBindParamDesc.relativeChangeFrequency = 0;
		depthTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc colorBindParamDesc;
		m_inputColorBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		colorBindParamDesc.parameter = m_inputColorBindParam;
		colorBindParamDesc.constantSize = 0;
		colorBindParamDesc.relativeAccessFrequency = 0;
		colorBindParamDesc.relativeChangeFrequency = 0;
		colorBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc cullRoBindParamDesc;
		m_cullRoBindParam = BindParameter(eBindParameterType::TEXTURE, 2);
		cullRoBindParamDesc.parameter = m_cullRoBindParam;
		cullRoBindParamDesc.constantSize = 0;
		cullRoBindParamDesc.relativeAccessFrequency = 0;
		cullRoBindParamDesc.relativeChangeFrequency = 0;
		cullRoBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc lightCullBindParamDesc;
		m_lightCullBindParam = BindParameter(eBindParameterType::TEXTURE, 3);
		lightCullBindParamDesc.parameter = m_lightCullBindParam;
		lightCullBindParamDesc.constantSize = 0;
		lightCullBindParamDesc.relativeAccessFrequency = 0;
		lightCullBindParamDesc.relativeChangeFrequency = 0;
		lightCullBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc csmTexBindParamDesc;
		m_csmTexBindParam = BindParameter(eBindParameterType::TEXTURE, 500);
		csmTexBindParamDesc.parameter = m_csmTexBindParam;
		csmTexBindParamDesc.constantSize = 0;
		csmTexBindParamDesc.relativeAccessFrequency = 0;
		csmTexBindParamDesc.relativeChangeFrequency = 0;
		csmTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc shadowMXTexBindParamDesc;
		m_shadowMxTexBindParam = BindParameter(eBindParameterType::TEXTURE, 501);
		shadowMXTexBindParamDesc.parameter = m_shadowMxTexBindParam;
		shadowMXTexBindParamDesc.constantSize = 0;
		shadowMXTexBindParamDesc.relativeAccessFrequency = 0;
		shadowMXTexBindParamDesc.relativeChangeFrequency = 0;
		shadowMXTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc csmSplitsTexBindParamDesc;
		m_csmSplitsTexBindParam = BindParameter(eBindParameterType::TEXTURE, 502);
		csmSplitsTexBindParamDesc.parameter = m_csmSplitsTexBindParam;
		csmSplitsTexBindParamDesc.constantSize = 0;
		csmSplitsTexBindParamDesc.relativeAccessFrequency = 0;
		csmSplitsTexBindParamDesc.relativeChangeFrequency = 0;
		csmSplitsTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc lightMvpTexBindParamDesc;
		m_lightMvpTexBindParam = BindParameter(eBindParameterType::TEXTURE, 503);
		lightMvpTexBindParamDesc.parameter = m_lightMvpTexBindParam;
		lightMvpTexBindParamDesc.constantSize = 0;
		lightMvpTexBindParamDesc.relativeAccessFrequency = 0;
		lightMvpTexBindParamDesc.relativeChangeFrequency = 0;
		lightMvpTexBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc shadowSampBindParamDesc;
		shadowSampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 500);
		shadowSampBindParamDesc.constantSize = 0;
		shadowSampBindParamDesc.relativeAccessFrequency = 0;
		shadowSampBindParamDesc.relativeChangeFrequency = 0;
		shadowSampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc theSamplerParam;
		theSamplerParam.shaderRegister = 500;
		theSamplerParam.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		theSamplerParam.addressU = gxapi::eTextureAddressMode::CLAMP;
		theSamplerParam.addressV = gxapi::eTextureAddressMode::CLAMP;
		theSamplerParam.addressW = gxapi::eTextureAddressMode::CLAMP;
		theSamplerParam.mipLevelBias = 0.f;
		theSamplerParam.registerSpace = 0;
		theSamplerParam.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc volDst0BindParamDesc;
		m_volDst0BindParam = BindParameter(eBindParameterType::UNORDERED, 0);
		volDst0BindParamDesc.parameter = m_volDst0BindParam;
		volDst0BindParamDesc.constantSize = 0;
		volDst0BindParamDesc.relativeAccessFrequency = 0;
		volDst0BindParamDesc.relativeChangeFrequency = 0;
		volDst0BindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc volDst1BindParamDesc;
		m_volDst1BindParam = BindParameter(eBindParameterType::UNORDERED, 1);
		volDst1BindParamDesc.parameter = m_volDst1BindParam;
		volDst1BindParamDesc.constantSize = 0;
		volDst1BindParamDesc.relativeAccessFrequency = 0;
		volDst1BindParamDesc.relativeChangeFrequency = 0;
		volDst1BindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc dstBindParamDesc;
		m_dstBindParam = BindParameter(eBindParameterType::UNORDERED, 2);
		dstBindParamDesc.parameter = m_dstBindParam;
		dstBindParamDesc.constantSize = 0;
		dstBindParamDesc.relativeAccessFrequency = 0;
		dstBindParamDesc.relativeChangeFrequency = 0;
		dstBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc cullBindParamDesc;
		m_cullBindParam = BindParameter(eBindParameterType::UNORDERED, 3);
		cullBindParamDesc.parameter = m_cullBindParam;
		cullBindParamDesc.constantSize = 0;
		cullBindParamDesc.relativeAccessFrequency = 0;
		cullBindParamDesc.relativeChangeFrequency = 0;
		cullBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, depthTexBindParamDesc, colorBindParamDesc, cullBindParamDesc, volDst0BindParamDesc, dstBindParamDesc, volDst1BindParamDesc, cullRoBindParamDesc, lightCullBindParamDesc, csmTexBindParamDesc, shadowMXTexBindParamDesc, csmSplitsTexBindParamDesc, lightMvpTexBindParamDesc, shadowSampBindParamDesc }, { samplerDesc, theSamplerParam });
	}

	if (!m_sdfCullingCSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.cs = true;

		{
			m_sdfCullingShader = context.CreateShader("SDFCulling", shaderParts, "");

			gxapi::ComputePipelineStateDesc csoDesc;
			csoDesc.rootSignature = m_binder.GetRootSignature();
			csoDesc.cs = m_sdfCullingShader.cs;

			m_sdfCullingCSO.reset(context.CreatePSO(csoDesc));
		}

		{
			m_volumetricLightingShader = context.CreateShader("VolumetricLighting", shaderParts, "");

			gxapi::ComputePipelineStateDesc csoDesc;
			csoDesc.rootSignature = m_binder.GetRootSignature();
			csoDesc.cs = m_volumetricLightingShader.cs;

			m_volumetricLightingCSO.reset(context.CreatePSO(csoDesc));
		}
	}

	this->GetOutput<0>().Set(m_dstTexUAV.GetResource());
}


void VolumetricLighting::Execute(RenderContext& context) {
	bool peti = true;
	if (peti) {
		return;
	}

	ComputeCommandList& commandList = context.AsCompute();

	//swap dest textures
	auto tmp = m_volDstTexUAV[0];
	m_volDstTexUAV[0] = m_volDstTexUAV[1];
	m_volDstTexUAV[1] = tmp;

	Uniforms uniformsCBData;

	uniformsCBData.camNear = m_camera->GetNearPlane();
	uniformsCBData.camFar = m_camera->GetFarPlane();
	uniformsCBData.numSdfs = 1;
	uniformsCBData.v = m_camera->GetViewMatrix();
	uniformsCBData.p = m_camera->GetProjectionMatrix();

	static uint32_t haltonIndex = 0;
	const uint32_t haltonBase = 2;
	uniformsCBData.haltonFactor = getHalton(haltonIndex++, haltonBase);

	uniformsCBData.camPos = Vec4(m_camera->GetPosition(), 1);

	uniformsCBData.sunDirection = Vec4(0.8f, -0.7f, -0.9f, 0.0f);
	uniformsCBData.sunColor = Vec4(1.0f, 0.9f, 0.85f, 1.0f);

	Mat44 VP = m_camera->GetViewMatrix() * m_camera->GetProjectionMatrix();
	Mat44 invVP = VP.Inverse();

	//worldVec: ndcVec * invVP
	//reprojNdcVec: worldVec * oldVP
	uniformsCBData.oldVP = m_prevVP;

	uniformsCBData.invVP = invVP;

	/*//far/near ndc corners
	Vec4 ndcCorners[] = 
	{
		Vec4(-1.f, -1.f, 0.f, 1.f),
		Vec4(-1.f, 1.f, 0.f, 1.f),
		Vec4(1.f, 1.f, 0.f, 1.f),
		Vec4(1.f, -1.f, 0.f, 1.f),
		Vec4(1.f, 1.f, 0.f, 1.f),
	};

	//convert to world space frustum corners
	ndcCorners[0] = ndcCorners[0] * invVP;
	ndcCorners[1] = ndcCorners[1] * invVP;
	ndcCorners[2] = ndcCorners[2] * invVP;
	ndcCorners[3] = ndcCorners[3] * invVP;
	ndcCorners[4] = ndcCorners[4] * invVP;
	ndcCorners[0] /= ndcCorners[0].w;
	ndcCorners[1] /= ndcCorners[1].w;
	ndcCorners[2] /= ndcCorners[2].w;
	ndcCorners[3] /= ndcCorners[3].w;
	ndcCorners[4] /= ndcCorners[4].w;

	uniformsCBData.near_plane_ll_corner = Vec4(ndcCorners[0].xyz, 1.0);
	uniformsCBData.near_plane_x_axis = Vec4((Vec3(ndcCorners[2].xyz) - Vec3(ndcCorners[1].xyz).Normalized()), 0.0f);
	uniformsCBData.near_plane_y_axis = Vec4((Vec3(ndcCorners[4].xyz) - Vec3(ndcCorners[3].xyz).Normalized()), 0.0f);
	uniformsCBData.near_plane_xy_sizes = Vec4((Vec3(ndcCorners[2].xyz) - Vec3(ndcCorners[1].xyz)).Length(), (Vec3(ndcCorners[4].xyz) - Vec3(ndcCorners[3].xyz)).Length(), 0, 0);*/

	//DebugDrawManager::GetInstance().AddSphere(Vec3(-0.5, 0, 1), 5.0f, 0);

	uint32_t dispatchW, dispatchH;
	SetWorkgroupSize((unsigned)m_depthTexSrv.GetResource().GetWidth(), (unsigned)m_depthTexSrv.GetResource().GetHeight(), 16, 16, dispatchW, dispatchH);

	uniformsCBData.numWorkgroupsX = dispatchW;
	uniformsCBData.numWorkgroupsY = dispatchH;

	{ //cull SDFs
		//view space for culling
		uniformsCBData.sd[0].vsPosition = Vec4(Vec3(-4.0, 0, 1), 1.0f) * m_camera->GetViewMatrix();
		uniformsCBData.sd[0].radius = 3.0f;

		//create single-frame only cb
		gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
		cb.SetName("SDF culling volatile CB");
		gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));


		commandList.SetResourceState(m_dstTexUAV.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_sdfCullDataUAV.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_volDstTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_volDstTexUAV[1].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_colorTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		commandList.SetPipelineState(m_sdfCullingCSO.get());
		commandList.SetComputeBinder(&m_binder);
		commandList.BindCompute(m_inputColorBindParam, m_colorTexSRV);
		commandList.BindCompute(m_cullBindParam, m_sdfCullDataUAV);
		commandList.BindCompute(m_dstBindParam, m_dstTexUAV);
		commandList.BindCompute(m_volDst0BindParam, m_volDstTexUAV[0]);
		commandList.BindCompute(m_volDst1BindParam, m_volDstTexUAV[1]);
		commandList.BindCompute(m_depthBindParam, m_depthTexSrv);
		commandList.BindCompute(m_uniformsBindParam, cbv);
		commandList.Dispatch(dispatchW, dispatchH, 1);
		commandList.UAVBarrier(m_sdfCullDataUAV.GetResource());
		commandList.UAVBarrier(m_volDstTexUAV[0].GetResource());
	}

	{ //do volumetric lighting
		//world space for lighting / raymarching
		uniformsCBData.sd[0].vsPosition = Vec4(Vec3(-4.0, 0, 1), 1.0f);
		uniformsCBData.sd[0].radius = 3.0f;

		uniformsCBData.ld[0].vsPosition = Vec4(Vec3(0, 0, 1), 1.0f);
		uniformsCBData.ld[0].attenuationEnd = 5.0f;
		uniformsCBData.ld[0].diffuseLightColor = Vec4(1.f, 0.f, 0.f, 1.f);

		//create single-frame only cb
		gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
		cb.SetName("SDF culling volatile CB");
		gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));


		commandList.SetResourceState(m_dstTexUAV.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_volDstTexUAV[0].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_volDstTexUAV[1].GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_colorTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_sdfCullDataSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_lightCullDataSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_csmTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_shadowMXTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_csmSplitsTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		commandList.SetPipelineState(m_volumetricLightingCSO.get());
		commandList.SetComputeBinder(&m_binder);
		commandList.BindCompute(m_csmTexBindParam, m_csmTexSRV);
		commandList.BindCompute(m_shadowMxTexBindParam, m_shadowMXTexSRV);
		commandList.BindCompute(m_csmSplitsTexBindParam, m_csmSplitsTexSRV);
		commandList.BindCompute(m_inputColorBindParam, m_colorTexSRV);
		commandList.BindCompute(m_cullRoBindParam, m_sdfCullDataSRV);
		commandList.BindCompute(m_lightCullBindParam, m_lightCullDataSRV);
		commandList.BindCompute(m_dstBindParam, m_dstTexUAV);
		commandList.BindCompute(m_volDst0BindParam, m_volDstTexUAV[0]);
		commandList.BindCompute(m_volDst1BindParam, m_volDstTexUAV[1]);
		commandList.BindCompute(m_depthBindParam, m_depthTexSrv);
		commandList.BindCompute(m_uniformsBindParam, cbv);
		commandList.Dispatch(dispatchW, dispatchH, 1);
		commandList.UAVBarrier(m_volDstTexUAV[0].GetResource());
	}

	m_prevVP = VP;
}


void VolumetricLighting::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatSDFCullData = eFormat::R32_UINT;
		auto formatDst = eFormat::R16G16B16A16_FLOAT;

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
		SetWorkgroupSize((unsigned)m_depthTexSrv.GetResource().GetWidth(), (unsigned)m_depthTexSrv.GetResource().GetHeight(), 16, 16, dispatchW, dispatchH);

		//TODO 1D tex
		Texture2DDesc desc;
		desc.width = dispatchW * dispatchH;
		desc.height = 1024;
		desc.format = formatSDFCullData;
		TextureUsage uavusage{ true, true, false, true };

		Texture2D sdfCullDataTex = context.CreateTexture2D(desc, uavusage);
		sdfCullDataTex.SetName("SDF culling sdf cull data tex");
		m_sdfCullDataUAV = context.CreateUav(sdfCullDataTex, formatSDFCullData, uavDesc);

		m_sdfCullDataSRV = context.CreateSrv(sdfCullDataTex, formatSDFCullData, srvDesc);


		for (int c = 0; c < 2; ++c) {
			desc.width = m_depthTexSrv.GetResource().GetWidth();
			desc.height = m_depthTexSrv.GetResource().GetHeight();
			desc.format = formatDst;

			Texture2D dstTex = context.CreateTexture2D(desc, uavusage);
			sdfCullDataTex.SetName("SDF culling dst tex");
			m_volDstTexUAV[c] = context.CreateUav(dstTex, formatDst, uavDesc);
			m_volDstTexUAV[c].GetResource().SetName((std::string("SDF culling vol dst UAV") + std::to_string(c)).c_str());
		}

		desc.width = m_depthTexSrv.GetResource().GetWidth();
		desc.height = m_depthTexSrv.GetResource().GetHeight();
		desc.format = formatDst;

		Texture2D dstTex = context.CreateTexture2D(desc, uavusage);
		sdfCullDataTex.SetName("SDF culling dst tex");
		m_dstTexUAV = context.CreateUav(dstTex, formatDst, uavDesc);
	}
}


} // namespace inl::gxeng::nodes
