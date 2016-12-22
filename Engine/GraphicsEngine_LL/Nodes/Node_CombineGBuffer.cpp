
#include "Node_CombineGBuffer.hpp"

#include "../DirectionalLight.hpp"

#include <array>

namespace inl::gxeng::nodes {


CombineGBuffer::CombineGBuffer(gxapi::IGraphicsApi* graphicsApi, unsigned width, unsigned height) :
	m_width(width),
	m_height(height),
	m_binder(graphicsApi, {}),
	m_fsq(nullptr),
	m_fsqIndices(nullptr)
{
	BindParameterDesc sunBindParamDesc;
	m_sunBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
	sunBindParamDesc.parameter = m_sunBindParam;
	sunBindParamDesc.constantSize = 0;
	sunBindParamDesc.relativeAccessFrequency = 0;
	sunBindParamDesc.relativeChangeFrequency = 0;
	sunBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc albedoRoughnessBindParamDesc;
	m_albedoRoughnessBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
	albedoRoughnessBindParamDesc.parameter = m_albedoRoughnessBindParam;
	albedoRoughnessBindParamDesc.constantSize = 0;
	albedoRoughnessBindParamDesc.relativeAccessFrequency = 0;
	albedoRoughnessBindParamDesc.relativeChangeFrequency = 0;
	albedoRoughnessBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc normalBindParamDesc;
	m_normalBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
	normalBindParamDesc.parameter = m_normalBindParam;
	normalBindParamDesc.constantSize = 0;
	normalBindParamDesc.relativeAccessFrequency = 0;
	normalBindParamDesc.relativeChangeFrequency = 0;
	normalBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

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

	m_binder = Binder{ graphicsApi,{ albedoRoughnessBindParamDesc, normalBindParamDesc, sampBindParamDesc },{ samplerDesc } };
}


void CombineGBuffer::InitGraphics(const GraphicsContext& context) {

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

	InitBuffer();

	ShaderParts shaderParts;
	shaderParts.vs = true;
	shaderParts.ps = true;

	auto shader = m_graphicsContext.CreateShader("CombineGBuffer.hlsl", shaderParts, "");

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
	//psoDesc.blending = BlendState();
	//psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
	//psoDesc.depthStencilFormat = gxapi::eFormat::D32_FLOAT;
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R16G16B16A16_FLOAT;

	m_PSO.reset(m_graphicsContext.CreatePSO(psoDesc));
}


void CombineGBuffer::Resize(unsigned width, unsigned height) {
	m_width = width;
	m_height = height;
	InitBuffer();
}


void CombineGBuffer::InitBuffer() {
	m_renderTarget = RenderTargetPack(m_width, m_height, gxapi::eFormat::R16G16B16A16_FLOAT, m_graphicsContext);
}


void CombineGBuffer::RenderCombined(
	Texture2DSRV& depthStencil,
	Texture2DSRV& albedoRoughness,
	Texture2DSRV& normal,
	const Camera* camera,
	const DirectionalLight* sun,
	GraphicsCommandList & commandList
) {
	// Set render target
	auto* pRTV = &m_renderTarget.rtv;
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

	mathfu::Matrix4x4f viewInvTr = camera->GetViewMatrixRH().Inverse().Transpose();
	mathfu::Vector4f sunViewDir = viewInvTr * mathfu::Vector4f(sun->GetDirection(), 0.0f);
	mathfu::Vector4f sunColor = mathfu::Vector4f(sun->GetColor(), 1.0f);

	std::array<mathfu::VectorPacked<float, 4>, 2> cbufferSun;
	sunViewDir.Pack(cbufferSun.data());
	sunColor.Pack(cbufferSun.data() + 1);

	gxeng::VertexBuffer* pVertexBuffer = &m_fsq;
	unsigned vbSize = m_fsq.GetSize();
	unsigned vbStride = 3*sizeof(float);

	commandList.BindGraphics(m_sunBindParam, cbufferSun.data(), sizeof(cbufferSun), 0);
	commandList.BindGraphics(m_albedoRoughnessBindParam, albedoRoughness);
	commandList.BindGraphics(m_normalBindParam, normal);
	commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
	commandList.SetIndexBuffer(&m_fsqIndices, false);
	commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
}


} // namespace inl::gxeng::nodes
