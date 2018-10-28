#include "LuminanceReductionFinal.hpp"

#include "../../Debug/DebugDrawManager.hpp"
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>


namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(LuminanceReductionFinal)

struct Uniforms {
	float middleGrey, deltaTime;
};


LuminanceReductionFinal::LuminanceReductionFinal() {
	this->GetInput<0>().Set({});
}


void LuminanceReductionFinal::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void LuminanceReductionFinal::Reset() {
	m_reductionTexSrv = TextureView2D();

	GetInput<0>().Clear();
}

const std::string& LuminanceReductionFinal::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"reductionTex"
	};
	return names[index];
}

const std::string& LuminanceReductionFinal::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"averageLuminance"
	};
	return names[index];
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


	this->GetOutput<0>().Set(m_avgLumUav.GetResource());

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
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.cs = true;

		m_shader = context.CreateShader("LuminanceReductionFinal", shaderParts, "");

		gxapi::ComputePipelineStateDesc csoDesc;
		csoDesc.rootSignature = m_binder.GetRootSignature();
		csoDesc.cs = m_shader.cs;

		m_CSO.reset(context.CreatePSO(csoDesc));
	}
}


void LuminanceReductionFinal::Execute(RenderContext& context) {
	ComputeCommandList& commandList = context.AsCompute();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	//TODO get from somewhere
	uniformsCBData.middleGrey = 0.1842f; //https://www.wikiwand.com/en/Middle_gray
	uniformsCBData.deltaTime = 0.16f; //seconds

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Depth reduction final volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	*/

	commandList.SetResourceState(m_avgLumUav.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_reductionTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	commandList.SetPipelineState(m_CSO.get());
	commandList.SetComputeBinder(&m_binder);
	commandList.BindCompute(m_reductionBindParam, m_reductionTexSrv);
	commandList.BindCompute(m_outputBindParam0, m_avgLumUav);
	commandList.BindCompute(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));
	commandList.Dispatch(1, 1, 1);
	commandList.UAVBarrier(m_avgLumUav.GetResource());
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

		Texture2D avgLumTex = context.CreateTexture2D({ 1, 1, formatAvgLum }, { true, true, false, true });
		avgLumTex.SetName("Luminance reduction final tex");
		m_avgLumUav = context.CreateUav(avgLumTex, formatAvgLum, uavDesc);
	}
}


} // namespace inl::gxeng::nodes
