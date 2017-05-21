#include "Node_BlendWithTransform.hpp"

#include "../GraphicsNode.hpp"

#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "../MemoryObject.hpp"
#include "../GraphicsCommandList.hpp"

#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <InlineMath.hpp>

namespace inl::gxeng::nodes {


void BlendWithTransform::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void BlendWithTransform::Reset() {
	m_fsq = {};
	m_fsqIndices = {};
	m_blendDest = {};
	m_blendSrc = {};
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}


void BlendWithTransform::Setup(SetupContext& context) {
	auto& target = this->GetInput<0>().Get();
	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_blendDest = context.CreateRtv(target, target.GetFormat(), rtvDesc);

	auto blendSrc = this->GetInput<1>().Get();
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;
	m_blendSrc = context.CreateSrv(blendSrc, blendSrc.GetFormat(), srvDesc);

	gxapi::RenderTargetBlendState currBlendMode = this->GetInput<2>().Get();

	m_transfrom = GetInput<3>().Get();

	this->GetOutput<0>().Set(target);


	if (!m_binder.has_value()) {
		BindParameterDesc transformParamDesc;
		m_transformParam = BindParameter(eBindParameterType::CONSTANT, 0);
		transformParamDesc.parameter = m_transformParam;
		transformParamDesc.constantSize = 4*4*sizeof(float);
		transformParamDesc.relativeAccessFrequency = 0;
		transformParamDesc.relativeChangeFrequency = 0;
		transformParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;
		
		BindParameterDesc tex0ParamDesc;
		m_tex0Param = BindParameter(eBindParameterType::TEXTURE, 0);
		tex0ParamDesc.parameter = m_tex0Param;
		tex0ParamDesc.constantSize = 0;
		tex0ParamDesc.relativeAccessFrequency = 0;
		tex0ParamDesc.relativeChangeFrequency = 0;
		tex0ParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

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

		m_binder = context.CreateBinder({ transformParamDesc, tex0ParamDesc, sampBindParamDesc }, { samplerDesc });
	}

	if (!m_fsq.HasObject() || !m_fsqIndices.HasObject()) {
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
		m_fsq = context.CreateVertexBuffer(vertices.data(), sizeof(float)*vertices.size());
		m_fsqIndices = context.CreateIndexBuffer(indices.data(), sizeof(uint16_t)*indices.size(), indices.size());
	}

	if (!m_shader.vs || m_shader.ps) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_shader = context.CreateShader("BlendWithTransform", shaderParts, "");
	}

	if (m_renderTargetFormat != target.GetFormat() || m_blendMode != currBlendMode) {
		m_renderTargetFormat = target.GetFormat();
		m_blendMode = currBlendMode;

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32_FLOAT, 0, 0)
		};

		gxapi::GraphicsPipelineStateDesc psoDesc;
		psoDesc.inputLayout.elements = inputElementDesc.data();
		psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
		psoDesc.rootSignature = m_binder->GetRootSignature();
		psoDesc.vs = m_shader.vs;
		psoDesc.ps = m_shader.ps;
		psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
		psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
		psoDesc.blending.alphaToCoverage = false;
		psoDesc.blending.independentBlending = false;
		psoDesc.blending.singleTarget = m_blendMode;

		psoDesc.numRenderTargets = 1;
		psoDesc.renderTargetFormats[0] = m_renderTargetFormat;

		m_PSO.reset(context.CreatePSO(psoDesc));
	}

}


void BlendWithTransform::Execute(RenderContext& context) {
	gxeng::GraphicsCommandList& commandList = context.AsGraphics();

	auto* pRTV = &m_blendDest;
	commandList.SetResourceState(pRTV->GetResource(), gxapi::eResourceState::RENDER_TARGET);
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
	commandList.SetGraphicsBinder(&m_binder.value());
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	Mat44_Packed transformPacked;

	transformPacked = m_transfrom;

	commandList.BindGraphics(m_transformParam, &transformPacked, sizeof(transformPacked));

	commandList.SetResourceState(const_cast<Texture2D&>(m_blendSrc.GetResource()), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.BindGraphics(m_tex0Param, m_blendSrc);

	assert(m_fsq.HasObject());

	gxeng::VertexBuffer* pVertexBuffer = &m_fsq;
	unsigned vbSize = (unsigned)m_fsq.GetSize();
	unsigned vbStride = 2 * sizeof(float);

	commandList.SetResourceState(*pVertexBuffer, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
	commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
	commandList.SetResourceState(m_fsqIndices, gxapi::eResourceState::INDEX_BUFFER);
	commandList.SetIndexBuffer(&m_fsqIndices, false);
	commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
}


} // namespace inl::gxeng::nodes
