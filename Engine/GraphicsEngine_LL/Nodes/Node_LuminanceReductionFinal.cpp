#include "Node_LuminanceReductionFinal.hpp"

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

struct Uniforms
{
	float middle_grey, delta_time;
};


LuminanceReductionFinal::LuminanceReductionFinal() {
	this->GetInput<0>().Set({});
}


void LuminanceReductionFinal::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void LuminanceReductionFinal::Reset() {
	m_reductionTexSrv = TextureView2D();

	GetInput<0>().Clear();
}


void LuminanceReductionFinal::Setup(SetupContext& context) {
	InitRenderTarget(context);


	Texture2D reductionTex = this->GetInput<0>().Get();

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;
	m_reductionTexSrv = context.CreateSrv(reductionTex, reductionTex.GetFormat(), srvDesc);
	m_reductionTexSrv.GetResource()._GetResourcePtr()->SetName("Luminance reduction final reduction tex SRV");

	this->GetOutput<0>().Set(m_avg_lum_uav.GetResource());

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

		BindParameterDesc reductionBindParamDesc;
		m_reductionBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		reductionBindParamDesc.parameter = m_reductionBindParam;
		reductionBindParamDesc.constantSize = 0;
		reductionBindParamDesc.relativeAccessFrequency = 0;
		reductionBindParamDesc.relativeChangeFrequency = 0;
		reductionBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc outputBindParamDesc0;
		m_outputBindParam0 = BindParameter(eBindParameterType::UNORDERED, 0);
		outputBindParamDesc0.parameter = m_outputBindParam0;
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
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.cs = true;

		m_shader = context.CreateShader("LuminanceReductionFinal", shaderParts, "");

		gxapi::ComputePipelineStateDesc csoDesc;
		csoDesc.rootSignature = m_binder->GetRootSignature();
		csoDesc.cs = m_shader.cs;

		m_CSO.reset(context.CreatePSO(csoDesc));
	}
}


void LuminanceReductionFinal::Execute(RenderContext& context) {
	ComputeCommandList& commandList = context.AsCompute();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	//TODO get from somewhere
	uniformsCBData.middle_grey = 0.1842; //https://www.wikiwand.com/en/Middle_gray
	uniformsCBData.delta_time = 0.16; //seconds

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb._GetResourcePtr()->SetName("Depth reduction final volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	cbv.GetResource()._GetResourcePtr()->SetName("Depth reduction final CBV");*/

	commandList.SetResourceState(m_avg_lum_uav.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_reductionTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	commandList.SetPipelineState(m_CSO.get());
	commandList.SetComputeBinder(&m_binder.value());
	commandList.BindCompute(m_reductionBindParam, m_reductionTexSrv);
	commandList.BindCompute(m_outputBindParam0, m_avg_lum_uav);
	commandList.BindCompute(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));
	commandList.Dispatch(1, 1, 1);
	commandList.UAVBarrier(m_avg_lum_uav.GetResource());
}


void LuminanceReductionFinal::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatAvgLum = eFormat::R16_FLOAT;

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

		Texture2D avg_lum_tex = context.CreateRWTexture2D(1, 1, formatAvgLum, 1);
		avg_lum_tex._GetResourcePtr()->SetName("Luminance reduction final tex");
		m_avg_lum_uav = context.CreateUav(avg_lum_tex, formatAvgLum, uavDesc);
		m_avg_lum_uav.GetResource()._GetResourcePtr()->SetName("Luminance reduction final UAV");
	}
}


} // namespace inl::gxeng::nodes
