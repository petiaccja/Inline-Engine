#pragma once


#include "../GraphicsNode.hpp"

#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "../GraphicsContext.hpp"

#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"
#include "Node_Blend.hpp"


namespace inl::gxeng::nodes {


Blend::Blend(gxapi::IGraphicsApi * graphicsApi, BlendMode mode) :
	m_mode(mode),
	m_binder(graphicsApi, {})
{
	BindParameterDesc tex0ParamDesc;
	m_tex0Param = BindParameter(eBindParameterType::TEXTURE, 0);
	tex0ParamDesc.parameter = m_tex0Param;
	tex0ParamDesc.constantSize = 0;
	tex0ParamDesc.relativeAccessFrequency = 0;
	tex0ParamDesc.relativeChangeFrequency = 0;
	tex0ParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc tex1ParamDesc;
	m_tex1Param = BindParameter(eBindParameterType::TEXTURE, 1);
	tex1ParamDesc.parameter = m_tex1Param;
	tex1ParamDesc.constantSize = 0;
	tex1ParamDesc.relativeAccessFrequency = 0;
	tex1ParamDesc.relativeChangeFrequency = 0;
	tex1ParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc sampBindParamDesc;
	sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
	sampBindParamDesc.constantSize = 0;
	sampBindParamDesc.relativeAccessFrequency = 0;
	sampBindParamDesc.relativeChangeFrequency = 0;
	sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	gxapi::StaticSamplerDesc samplerDesc;
	samplerDesc.shaderRegister = 0;
	samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
	samplerDesc.mipLevelBias = 0.f;
	samplerDesc.registerSpace = 0;
	samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	m_binder = Binder{ graphicsApi,{ tex0ParamDesc, tex1ParamDesc, sampBindParamDesc },{ samplerDesc } };
}


void Blend::InitGraphics(const GraphicsContext& context) {
	m_graphicsContext = context;

	std::vector<float> vertices = {
		-1, -1, 
		+1, -1, 
		+1, +1, 
		-1, +1
	};
	std::vector<uint16_t> indices = {
		0, 1, 2,
		0, 2, 3
	};
	m_fsq = m_graphicsContext.CreateVertexBuffer(vertices.data(), sizeof(float)*vertices.size());
	m_fsqIndices = m_graphicsContext.CreateIndexBuffer(indices.data(), sizeof(uint16_t)*indices.size(), indices.size());

	ShaderParts shaderParts;
	shaderParts.vs = true;
	shaderParts.ps = true;

	std::string shaderName;

	switch (m_mode) {
	case CASUAL_ALPHA_BLEND:
		shaderName = "Blend_CasualAlpha";
		break;
	default:
		assert(false);
		break;
	}

	auto shader = m_graphicsContext.CreateShader(shaderName, shaderParts, "");

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32_FLOAT, 0, 0)
	};

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = m_binder.GetRootSignature();
	psoDesc.vs = shader.vs;
	psoDesc.ps = shader.ps;
	psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = COLOR_FORMAT;

	m_PSO.reset(m_graphicsContext.CreatePSO(psoDesc));
}


Task Blend::GetTask() {
	return Task({ [this](const ExecutionContext& context) {
		ExecutionResult result;

		auto target = this->GetInput<0>().Get();
		this->GetInput<0>().Clear();

		auto texture0 = this->GetInput<1>().Get();
		this->GetInput<1>().Clear();

		auto texture1 = this->GetInput<2>().Get();
		this->GetInput<2>().Clear();

		GraphicsCommandList cmdList = context.GetGraphicsCommandList();
		Render(target.QueryRenderTarget(cmdList, m_graphicsContext), texture0.QueryRead(), texture1.QueryRead(), cmdList);
		result.AddCommandList(std::move(cmdList));

		this->GetOutput<0>().Set(target);

		return result;
	} });
}


void Blend::Render(
	const RenderTargetView2D& target,
	const TextureView2D& texture0,
	const TextureView2D& texture1,
	GraphicsCommandList& commandList)
{
	auto* pRTV = &target;
	commandList.SetResourceState(pRTV->GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &pRTV);

	gxapi::Rectangle rect{ 0, (int)pRTV->GetResource().GetHeight(), 0, (int)pRTV->GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder);
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	gxeng::VertexBuffer* pVertexBuffer = &m_fsq;
	unsigned vbSize = (unsigned)m_fsq.GetSize();
	unsigned vbStride = 2 * sizeof(float);

	commandList.SetResourceState(const_cast<Texture2D&>(texture0.GetResource()), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
	commandList.BindGraphics(m_tex0Param, texture0);
	commandList.SetResourceState(const_cast<Texture2D&>(texture1.GetResource()), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
	commandList.BindGraphics(m_tex1Param, texture1);

	commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
	commandList.SetIndexBuffer(&m_fsqIndices, false);
	commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
}

} // namespace inl::gxeng::nodes
