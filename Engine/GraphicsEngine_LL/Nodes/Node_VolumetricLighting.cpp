#include "Node_VolumetricLighting.hpp"

#include "NodeUtility.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../PerspectiveCamera.hpp"
#include "../GraphicsCommandList.hpp"
#include "../EntityCollection.hpp"

#include "DebugDrawManager.hpp"

#include <array>

namespace inl::gxeng::nodes {

struct sdf_data
{
	Vec4_Packed vs_position;
	float radius;
	Vec3_Packed dummy;
};

struct Uniforms
{
	sdf_data sd[10];
	Mat44_Packed p;
	Vec4_Packed far_plane0, far_plane1;
	float cam_near, cam_far;
	uint32_t num_sdfs, num_workgroups_x, num_workgroups_y;
	Vec3_Packed dummy;
};

static void SetWorkgroupSize(unsigned w, unsigned h, unsigned groupSizeW, unsigned groupSizeH, unsigned& dispatchW, unsigned& dispatchH)
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


VolumetricLighting::VolumetricLighting() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
}


void VolumetricLighting::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void VolumetricLighting::Reset() {
	m_depthTexSrv = TextureView2D();
	m_sdfCullDataUAV = RWTextureView2D();
	m_colorTexSRV = TextureView2D();
	m_dstTexUAV = RWTextureView2D();
	m_camera = nullptr;

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
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
	m_depthTexSrv.GetResource()._GetResourcePtr()->SetName("SDF culling depth tex view");

	gxapi::UavTexture2DArray uavDesc;
	uavDesc.activeArraySize = 1;
	uavDesc.firstArrayElement = 0;
	uavDesc.mipLevel = 0;
	uavDesc.planeIndex = 0;

	Texture2D colorTex = this->GetInput<1>().Get();
	m_colorTexSRV = context.CreateSrv(colorTex, colorTex.GetFormat(), srvDesc);
	m_colorTexSRV.GetResource()._GetResourcePtr()->SetName("SDF culling color tex srv");

	m_camera = this->GetInput<2>().Get();

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

		BindParameterDesc dstBindParamDesc;
		m_dstBindParam = BindParameter(eBindParameterType::UNORDERED, 0);
		dstBindParamDesc.parameter = m_dstBindParam;
		dstBindParamDesc.constantSize = 0;
		dstBindParamDesc.relativeAccessFrequency = 0;
		dstBindParamDesc.relativeChangeFrequency = 0;
		dstBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc cullBindParamDesc;
		m_cullBindParam = BindParameter(eBindParameterType::UNORDERED, 1);
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

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, depthTexBindParamDesc, colorBindParamDesc, cullBindParamDesc, dstBindParamDesc },{ samplerDesc });
	}

	if (!m_sdfCullingCSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.cs = true;

		{
			m_sdfCullingShader = context.CreateShader("SDFCulling", shaderParts, "");

			gxapi::ComputePipelineStateDesc csoDesc;
			csoDesc.rootSignature = m_binder->GetRootSignature();
			csoDesc.cs = m_sdfCullingShader.cs;

			m_sdfCullingCSO.reset(context.CreatePSO(csoDesc));
		}

		{
			m_volumetricLightingShader = context.CreateShader("VolumetricLighting", shaderParts, "");

			gxapi::ComputePipelineStateDesc csoDesc;
			csoDesc.rootSignature = m_binder->GetRootSignature();
			csoDesc.cs = m_volumetricLightingShader.cs;

			m_volumetricLightingCSO.reset(context.CreatePSO(csoDesc));
		}
	}

	this->GetOutput<0>().Set(m_dstTexUAV.GetResource());
}


void VolumetricLighting::Execute(RenderContext& context) {
	ComputeCommandList& commandList = context.AsCompute();

	Uniforms uniformsCBData;

	uniformsCBData.cam_near = m_camera->GetNearPlane();
	uniformsCBData.cam_far = m_camera->GetFarPlane();
	uniformsCBData.num_sdfs = 1;
	uniformsCBData.p = m_camera->GetProjectionMatrix();

	Mat44 invVP = (m_camera->GetViewMatrix() * m_camera->GetProjectionMatrix()).Inverse();

	//near ndc corners
	Vec4 ndcCorners[] = 
	{
		Vec4(-1.f, -1.f, 0.f, 1.f),
		Vec4(1.f, 1.f, 0.f, 1.f),
	};

	//convert to world space frustum corners
	ndcCorners[0] = ndcCorners[0] * invVP;
	ndcCorners[1] = ndcCorners[1] * invVP;
	ndcCorners[0] /= ndcCorners[0].w;
	ndcCorners[1] /= ndcCorners[1].w;

	uniformsCBData.far_plane0 = Vec4(ndcCorners[0].xyz, ndcCorners[1].x);
	uniformsCBData.far_plane1 = Vec4(ndcCorners[1].y, ndcCorners[1].z, 0.0f, 0.0f);

	uniformsCBData.sd[0].vs_position = Vec4(Vec3(-0.5, 0, 1), 1.0f) * m_camera->GetViewMatrix();
	//uniformsCBData.ld[0].vs_position = Vec4(m_camera->GetPosition() + m_camera->GetLookDirection() * 5.f, 1.0f) * m_camera->GetViewMatrix();
	uniformsCBData.sd[0].radius = 5.0f;

	//DebugDrawManager::GetInstance().AddSphere(Vec3(-0.5, 0, 1), 5.0f, 0);

	uint32_t dispatchW, dispatchH;
	SetWorkgroupSize((unsigned)m_depthTexSrv.GetResource().GetWidth(), (unsigned)m_depthTexSrv.GetResource().GetHeight(), 16, 16, dispatchW, dispatchH);

	uniformsCBData.num_workgroups_x = dispatchW;
	uniformsCBData.num_workgroups_y = dispatchH;

	//create single-frame only cb
	gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb._GetResourcePtr()->SetName("SDF culling volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	cbv.GetResource()._GetResourcePtr()->SetName("SDF culling CBV");

	{ //cull SDFs
		commandList.SetResourceState(m_sdfCullDataUAV.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_dstTexUAV.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_colorTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		commandList.SetPipelineState(m_sdfCullingCSO.get());
		commandList.SetComputeBinder(&m_binder.value());
		commandList.BindCompute(m_inputColorBindParam, m_colorTexSRV);
		commandList.BindCompute(m_cullBindParam, m_sdfCullDataUAV);
		commandList.BindCompute(m_dstBindParam, m_dstTexUAV);
		commandList.BindCompute(m_depthBindParam, m_depthTexSrv);
		commandList.BindCompute(m_uniformsBindParam, cbv);
		commandList.Dispatch(dispatchW, dispatchH, 1);
		commandList.UAVBarrier(m_sdfCullDataUAV.GetResource());
		commandList.UAVBarrier(m_dstTexUAV.GetResource());
	}

	{ //do volumetric lighting
		commandList.SetResourceState(m_sdfCullDataUAV.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_dstTexUAV.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
		commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_colorTexSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		commandList.SetPipelineState(m_volumetricLightingCSO.get());
		commandList.SetComputeBinder(&m_binder.value());
		commandList.BindCompute(m_inputColorBindParam, m_colorTexSRV);
		commandList.BindCompute(m_cullBindParam, m_sdfCullDataUAV);
		commandList.BindCompute(m_dstBindParam, m_dstTexUAV);
		commandList.BindCompute(m_depthBindParam, m_depthTexSrv);
		commandList.BindCompute(m_uniformsBindParam, cbv);
		commandList.Dispatch(dispatchW, dispatchH, 1);
		commandList.UAVBarrier(m_sdfCullDataUAV.GetResource());
		commandList.UAVBarrier(m_dstTexUAV.GetResource());
	}
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
		Texture2D sdfCullDataTex = context.CreateRWTexture2D(dispatchW * dispatchH, 1024, formatSDFCullData, 1);
		sdfCullDataTex._GetResourcePtr()->SetName("SDF culling sdf cull data tex");
		m_sdfCullDataUAV = context.CreateUav(sdfCullDataTex, formatSDFCullData, uavDesc);
		m_sdfCullDataUAV.GetResource()._GetResourcePtr()->SetName("SDF culling sdf cull data UAV");

		Texture2D dstTex = context.CreateRWTexture2D(m_depthTexSrv.GetResource().GetWidth(), m_depthTexSrv.GetResource().GetHeight(), formatDst, 1);
		sdfCullDataTex._GetResourcePtr()->SetName("SDF culling dst tex");
		m_dstTexUAV = context.CreateUav(dstTex, formatDst, uavDesc);
		m_dstTexUAV.GetResource()._GetResourcePtr()->SetName("SDF culling dst UAV");
	}
}


} // namespace inl::gxeng::nodes
