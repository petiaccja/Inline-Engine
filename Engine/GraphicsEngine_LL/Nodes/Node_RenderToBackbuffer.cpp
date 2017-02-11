#include "Node_RenderToBackBuffer.hpp"


namespace inl::gxeng::nodes {


RenderToBackBuffer::RenderToBackBuffer(gxapi::IGraphicsApi * graphicsApi):
	m_binder(graphicsApi, {})
{
	BindParameterDesc texBindParamDesc;
	m_texBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
	texBindParamDesc.parameter = m_texBindParam;
	texBindParamDesc.constantSize = 0;
	texBindParamDesc.relativeAccessFrequency = 0;
	texBindParamDesc.relativeChangeFrequency = 0;
	texBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

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

	m_binder = Binder{ graphicsApi,{ texBindParamDesc, sampBindParamDesc },{ samplerDesc } };
}


void RenderToBackBuffer::InitGraphics(const GraphicsContext& context) {
	m_graphicsContext = context;

	std::vector<float> vertices = {
		-1, -1, 0,
		+1, -1, 0,
		+1, +1, 0,
		-1, +1, 0
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

	auto shader = m_graphicsContext.CreateShader("RenderToBackBuffer", shaderParts, "");

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0)
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
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R8G8B8A8_UNORM;

	m_PSO.reset(m_graphicsContext.CreatePSO(psoDesc));
}


void RenderToBackBuffer::Render(RenderTargetView2D& rtv, const TextureView2D& texture, GraphicsCommandList& commandList) {
	auto* pRTV = &rtv;
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
	unsigned vbStride = 3 * sizeof(float);

	commandList.SetResourceState(const_cast<Texture2D&>(texture.GetResource()), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
	commandList.BindGraphics(m_texBindParam, texture);
	commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
	commandList.SetIndexBuffer(&m_fsqIndices, false);
	commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
}


} // namespace inl::gxeng::nodes
