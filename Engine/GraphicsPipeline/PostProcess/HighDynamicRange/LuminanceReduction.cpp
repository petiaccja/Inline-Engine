#include "LuminanceReduction.hpp"

#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>



namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(LuminanceReduction)


static void setWorkgroupSize(unsigned w, unsigned h, unsigned groupSizeW, unsigned groupSizeH, unsigned& dispatchW, unsigned& dispatchH) {
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


LuminanceReduction::LuminanceReduction() : m_width(0), m_height(0) {
	this->GetInput<0>().Set({});
}


void LuminanceReduction::Initialize(EngineContext& context) {
	SetTaskSingle(this);
}

void LuminanceReduction::Reset() {
	m_luminanceView = {};
	m_uav = {};
	m_srv = {};
	GetInput(0)->Clear();
}

const std::string& LuminanceReduction::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"luminanceTex"
	};
	return names[index];
}

const std::string& LuminanceReduction::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"reductionTex"
	};
	return names[index];
}

void LuminanceReduction::Setup(SetupContext& context) {
	auto& inputLuminance = this->GetInput<0>().Get();
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;
	m_luminanceView = context.CreateSrv(inputLuminance, inputLuminance.GetFormat(), srvDesc);


	if (inputLuminance.GetWidth() != m_width || inputLuminance.GetHeight() != m_height || !m_srv) {
		m_width = inputLuminance.GetWidth();
		m_height = inputLuminance.GetHeight();
		InitRenderTarget(context);
	}

	this->GetOutput<0>().Set(m_srv.GetResource());

	if (!m_binder) {
		BindParameterDesc sampBindParamDesc;
		sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
		sampBindParamDesc.constantSize = 0;
		sampBindParamDesc.relativeAccessFrequency = 0;
		sampBindParamDesc.relativeChangeFrequency = 0;
		sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc luminanceBindParamDesc;
		m_luminanceBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		luminanceBindParamDesc.parameter = m_luminanceBindParam;
		luminanceBindParamDesc.constantSize = 0;
		luminanceBindParamDesc.relativeAccessFrequency = 0;
		luminanceBindParamDesc.relativeChangeFrequency = 0;
		luminanceBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc outputBindParamDesc;
		m_outputBindParam = BindParameter(eBindParameterType::UNORDERED, 0);
		outputBindParamDesc.parameter = m_outputBindParam;
		outputBindParamDesc.constantSize = 0;
		outputBindParamDesc.relativeAccessFrequency = 0;
		outputBindParamDesc.relativeChangeFrequency = 0;
		outputBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ sampBindParamDesc, luminanceBindParamDesc, outputBindParamDesc }, { samplerDesc });
	}

	if (m_CSO == nullptr) {
		ShaderParts shaderParts;
		shaderParts.cs = true;

		m_shader = context.CreateShader("LuminanceReduction", shaderParts, "");

		gxapi::ComputePipelineStateDesc csoDesc;
		csoDesc.rootSignature = m_binder.GetRootSignature();
		csoDesc.cs = m_shader.cs;

		m_CSO.reset(context.CreatePSO(csoDesc));
	}
}


void LuminanceReduction::Execute(RenderContext& context) {
	auto& commandList = context.AsCompute();

	unsigned dispatchW, dispatchH;
	setWorkgroupSize((unsigned)std::ceil(m_width * 0.5f), m_height, 16, 16, dispatchW, dispatchH);

	commandList.SetResourceState(m_uav.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_luminanceView.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	commandList.SetPipelineState(m_CSO.get());
	commandList.SetComputeBinder(&m_binder);
	commandList.BindCompute(m_luminanceBindParam, m_luminanceView);
	commandList.BindCompute(m_outputBindParam, m_uav);
	commandList.Dispatch(dispatchW, dispatchH, 1);
	commandList.UAVBarrier(m_uav.GetResource());
}


void LuminanceReduction::InitRenderTarget(SetupContext& context) {
	using gxapi::eFormat;

	auto formatLuminanceReductionResult = eFormat::R16_FLOAT;

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

	unsigned dispatchW, dispatchH;
	setWorkgroupSize((unsigned)std::ceil(m_width * 0.5f), m_height, 16, 16, dispatchW, dispatchH);

	Texture2D tex = context.CreateTexture2D({ dispatchW, dispatchH, formatLuminanceReductionResult }, { true, true, false, true });
	tex.SetName("Luminance reduction intermediate texture");
	m_uav = context.CreateUav(tex, formatLuminanceReductionResult, uavDesc);

	m_srv = context.CreateSrv(tex, formatLuminanceReductionResult, srvDesc);
}


} // namespace inl::gxeng::nodes
