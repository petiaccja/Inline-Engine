#include "Node_DepthReduction.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../GraphicsCommandList.hpp"

#include <array>

namespace inl::gxeng::nodes {

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


DepthReduction::DepthReduction(gxapi::IGraphicsApi * graphicsApi):
	m_binder(graphicsApi, {}), m_width(0), m_height(0)
{
	this->GetInput<0>().Set({});

	BindParameterDesc sampBindParamDesc;
	sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
	sampBindParamDesc.constantSize = 0;
	sampBindParamDesc.relativeAccessFrequency = 0;
	sampBindParamDesc.relativeChangeFrequency = 0;
	sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

	BindParameterDesc depthBindParamDesc;
	m_depthBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
	depthBindParamDesc.parameter = m_depthBindParam;
	depthBindParamDesc.constantSize = 0;
	depthBindParamDesc.relativeAccessFrequency = 0;
	depthBindParamDesc.relativeChangeFrequency = 0;
	depthBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

	BindParameterDesc outputBindParamDesc;
	m_outputBindParam = BindParameter(eBindParameterType::UNORDERED, 0);
	outputBindParamDesc.parameter = m_outputBindParam;
	outputBindParamDesc.constantSize = 0;
	outputBindParamDesc.relativeAccessFrequency = 0;
	outputBindParamDesc.relativeChangeFrequency = 0;
	outputBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

	gxapi::StaticSamplerDesc samplerDesc;
	samplerDesc.shaderRegister = 0;
	samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
	samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.mipLevelBias = 0.f;
	samplerDesc.registerSpace = 0;
	samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

	m_binder = Binder{ graphicsApi,{ sampBindParamDesc, depthBindParamDesc, outputBindParamDesc },{ samplerDesc } };
}


void DepthReduction::Initialize(EngineContext & context) {}


void DepthReduction::Setup(SetupContext& context) {
	if (m_CSO == nullptr) {
		ShaderParts shaderParts;
		shaderParts.cs = true;

		auto shader = context.CreateShader("DepthReduction", shaderParts, "");

		gxapi::ComputePipelineStateDesc csoDesc;
		csoDesc.rootSignature = m_binder.GetRootSignature();
		csoDesc.cs = shader.cs;

		m_CSO.reset(context.CreatePSO(csoDesc));
	}

	auto& inputDepth = this->GetInput<0>().Get();

	gxapi::SrvTexture2DArray desc;
	desc.activeArraySize = 1;
	desc.firstArrayElement = 0;
	desc.mipLevelClamping = 0;
	desc.mostDetailedMip = 0;
	desc.numMipLevels = 1;
	desc.planeIndex = 0;

	m_depthView = context.CreateSrv(inputDepth, inputDepth.GetFormat(), desc);
	this->GetInput<0>().Clear();

	if (inputDepth.GetWidth() != m_width || inputDepth.GetHeight() != m_height) {
		m_width = inputDepth.GetWidth();
		m_height = inputDepth.GetHeight();
		InitRenderTarget(context);
	}

	this->GetOutput<0>().Set(m_srv.GetResource());
}


void DepthReduction::Execute(RenderContext & context) {
	auto& commandList = context.AsCompute();

	unsigned dispatchW, dispatchH;
	setWorkgroupSize((unsigned)std::ceil(m_width * 0.5f), m_height, 16, 16, dispatchW, dispatchH);

	commandList.SetPipelineState(m_CSO.get());
	commandList.SetComputeBinder(&m_binder);
	commandList.BindCompute(m_depthBindParam, m_depthView);
	commandList.BindCompute(m_outputBindParam, m_uav);
	commandList.Dispatch(dispatchW, dispatchH, 1);
	commandList.UAVBarrier(m_uav.GetResource());
}


void DepthReduction::InitRenderTarget(SetupContext& context) {
	using gxapi::eFormat;

	auto formatDepthReductionResult = eFormat::R32G32_FLOAT;

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

	Texture2D tex = context.CreateRWTexture2D(dispatchW, dispatchH, formatDepthReductionResult, 1);
	m_uav = context.CreateUav(tex, formatDepthReductionResult, uavDesc);
	m_srv = context.CreateSrv(tex, formatDepthReductionResult, srvDesc);
}


} // namespace inl::gxeng::nodes
