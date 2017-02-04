#include "Node_DepthReduction.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"

#include <array>

namespace inl::gxeng::nodes {


DepthReduction::DepthReduction(gxapi::IGraphicsApi * graphicsApi, unsigned width, unsigned height):
	m_binder(graphicsApi, {}),
	m_width(width),
	m_height(height)
{
	this->GetInput<0>().Set({});

	BindParameterDesc sampBindParamDesc;
	sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
	sampBindParamDesc.constantSize = 0;
	sampBindParamDesc.relativeAccessFrequency = 0;
	sampBindParamDesc.relativeChangeFrequency = 0;
	sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	gxapi::StaticSamplerDesc samplerDesc;
	samplerDesc.shaderRegister = 0;
	samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
	samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.mipLevelBias = 0.f;
	samplerDesc.registerSpace = 0;
	samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	m_binder = Binder{ graphicsApi,{ sampBindParamDesc },{ samplerDesc } };
}


void DepthReduction::InitGraphics(const GraphicsContext & context) {
	m_graphicsContext = context;

	InitRenderTarget();

	ShaderParts shaderParts;
	shaderParts.cs = true;

	auto shader = m_graphicsContext.CreateShader("DepthReduction", shaderParts, "");

	gxapi::ComputePipelineStateDesc csoDesc;
	csoDesc.rootSignature = m_binder.GetRootSignature();
	csoDesc.cs = shader.cs;

	m_CSO.reset(m_graphicsContext.CreatePSO(csoDesc));
}


Task DepthReduction::GetTask() {
	return Task({ [this](const ExecutionContext& context) {
		ExecutionResult result;

		gxeng::pipeline::Texture2D depthTex = this->GetInput<0>().Get();
		this->GetInput<0>().Clear();

		this->GetOutput<0>().Set(m_uav);

		{
			ComputeCommandList cmdList = context.GetComputeCommandList();

			RenderScene(m_uav, depthTex, cmdList);
			result.AddCommandList(std::move(cmdList));
		}

		return result;
	} });
}


void DepthReduction::WindowResized(unsigned width, unsigned height) {
	m_width = width;
	m_height = height;
	InitRenderTarget();
}


void DepthReduction::InitRenderTarget() {
	using gxapi::eFormat;

	auto formatDepthReductionResult = eFormat::R32G32B32A32_FLOAT;
	auto formatColor = eFormat::R32_FLOAT_X8X24_TYPELESS;
	auto formatTypeless = eFormat::R32G8X24_TYPELESS;

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.numMipLevels = -1;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.planeIndex = 0;

	Texture2D tex = m_graphicsContext.CreateTexture2D(m_width, m_height, formatDepthReductionResult, 1);
	//m_graphicsContext
}


void DepthReduction::RenderScene(
	const gxeng::RWTextureView2D& uav,
	pipeline::Texture2D& depthTex,
	ComputeCommandList& commandList
) {
	commandList.BindCompute(gxeng::BindParameter(), uav);
	commandList.Dispatch(1, 1, 1);
	commandList.ResourceBarrier();
}


} // namespace inl::gxeng::nodes
