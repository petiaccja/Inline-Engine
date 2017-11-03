#include "Node_DOFPrepare.hpp"

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
	float maxBlurDiameter;
};


DOFPrepare::DOFPrepare() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
	this->GetInput<2>().Set({});
}


void DOFPrepare::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void DOFPrepare::Reset() {
	m_inputTexSrv = TextureView2D();
	m_depthTexSrv = TextureView2D();
	m_camera = nullptr;

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
}


void DOFPrepare::Setup(SetupContext& context) {
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	Texture2D inputTex = this->GetInput<0>().Get();
	m_inputTexSrv = context.CreateSrv(inputTex, inputTex.GetFormat(), srvDesc);
	m_inputTexSrv.GetResource()._GetResourcePtr()->SetName("DOF input tex SRV");

	Texture2D depthTex = this->GetInput<1>().Get();
	m_depthTexSrv = context.CreateSrv(depthTex, FormatDepthToColor(depthTex.GetFormat()), srvDesc);
	m_depthTexSrv.GetResource()._GetResourcePtr()->SetName("DOF depth tex SRV");

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

		BindParameterDesc sampBindParamDesc2;
		sampBindParamDesc2.parameter = BindParameter(eBindParameterType::SAMPLER, 1);
		sampBindParamDesc2.constantSize = 0;
		sampBindParamDesc2.relativeAccessFrequency = 0;
		sampBindParamDesc2.relativeChangeFrequency = 0;
		sampBindParamDesc2.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc inputBindParamDesc;
		m_inputTexBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		inputBindParamDesc.parameter = m_inputTexBindParam;
		inputBindParamDesc.constantSize = 0;
		inputBindParamDesc.relativeAccessFrequency = 0;
		inputBindParamDesc.relativeChangeFrequency = 0;
		inputBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc depthBindParamDesc;
		m_depthTexBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		depthBindParamDesc.parameter = m_depthTexBindParam;
		depthBindParamDesc.constantSize = 0;
		depthBindParamDesc.relativeAccessFrequency = 0;
		depthBindParamDesc.relativeChangeFrequency = 0;
		depthBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc2;
		samplerDesc2.shaderRegister = 1;
		samplerDesc2.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samplerDesc2.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc2.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc2.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc2.mipLevelBias = 0.f;
		samplerDesc2.registerSpace = 0;
		samplerDesc2.shaderVisibility = gxapi::eShaderVisiblity::ALL;


		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, sampBindParamDesc2, inputBindParamDesc, depthBindParamDesc },{ samplerDesc, samplerDesc2 });
	}

	if (!m_fsq.HasObject()) {
		std::vector<float> vertices = {
			-1, -1, 0,  0, +1,
			+1, -1, 0, +1, +1,
			+1, +1, 0, +1,  0,
			-1, +1, 0,  0,  0
		};
		std::vector<uint16_t> indices = {
			0, 1, 2,
			0, 2, 3
		};
		m_fsq = context.CreateVertexBuffer(vertices.data(), sizeof(float)*vertices.size());
		m_fsq._GetResourcePtr()->SetName("DOF full screen quad vertex buffer");
		m_fsqIndices = context.CreateIndexBuffer(indices.data(), sizeof(uint16_t)*indices.size(), indices.size());
		m_fsqIndices._GetResourcePtr()->SetName("DOF full screen quad index buffer");
	}

	if (!m_PSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_shader = context.CreateShader("DOFPrepare", shaderParts, "");

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
		};

		gxapi::GraphicsPipelineStateDesc psoDesc;
		psoDesc.inputLayout.elements = inputElementDesc.data();
		psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
		psoDesc.rootSignature = m_binder->GetRootSignature();
		psoDesc.vs = m_shader.vs;
		psoDesc.ps = m_shader.ps;
		psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
		psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

		psoDesc.depthStencilState.enableDepthTest = false;
		psoDesc.depthStencilState.enableDepthStencilWrite = false;
		psoDesc.depthStencilState.enableStencilTest = false;
		psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

		psoDesc.numRenderTargets = 2;
		psoDesc.renderTargetFormats[0] = m_prepare_rtv.GetResource().GetFormat();
		psoDesc.renderTargetFormats[1] = m_depth_rtv.GetResource().GetFormat();

		m_PSO.reset(context.CreatePSO(psoDesc));
	}

	this->GetOutput<0>().Set(m_prepare_rtv.GetResource());
	this->GetOutput<1>().Set(m_depth_rtv.GetResource());
}


void DOFPrepare::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb._GetResourcePtr()->SetName("Bright Lum pass volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	cbv.GetResource()._GetResourcePtr()->SetName("Bright Lum pass CBV");*/

	uniformsCBData.maxBlurDiameter = 33.0;

	commandList.SetResourceState(m_prepare_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_depth_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_inputTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	float fnear = m_camera->GetNearPlane();
	float ffar = m_camera->GetFarPlane();

	RenderTargetView2D* pRTV[2] = { &m_prepare_rtv, &m_depth_rtv };
	commandList.SetRenderTargets(2, pRTV, 0);

	gxapi::Rectangle rect{ 0, (int)m_prepare_rtv.GetResource().GetHeight(), 0, (int)m_prepare_rtv.GetResource().GetWidth() };
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

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder.value());
	commandList.BindGraphics(m_inputTexBindParam, m_inputTexSrv);
	commandList.BindGraphics(m_depthTexBindParam, m_depthTexSrv);
	commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

	gxeng::VertexBuffer* pVertexBuffer = &m_fsq;
	unsigned vbSize = (unsigned)m_fsq.GetSize();
	unsigned vbStride = 5 * sizeof(float);

	commandList.SetResourceState(*pVertexBuffer, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
	commandList.SetResourceState(m_fsqIndices, gxapi::eResourceState::INDEX_BUFFER);
	commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
	commandList.SetIndexBuffer(&m_fsqIndices, false);
	commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
}


void DOFPrepare::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto format = eFormat::R16G16B16A16_FLOAT;
		auto depthFormat = eFormat::R32_FLOAT;

		gxapi::RtvTexture2DArray rtvDesc;
		rtvDesc.activeArraySize = 1;
		rtvDesc.firstArrayElement = 0;
		rtvDesc.firstMipLevel = 0;
		rtvDesc.planeIndex = 0;

		gxapi::SrvTexture2DArray srvDesc;
		srvDesc.activeArraySize = 1;
		srvDesc.firstArrayElement = 0;
		srvDesc.numMipLevels = -1;
		srvDesc.mipLevelClamping = 0;
		srvDesc.mostDetailedMip = 0;
		srvDesc.planeIndex = 0;

		//Texture2D prepare_tex = context.CreateTexture2D(m_inputTexSrv.GetResource().GetWidth()/2, m_inputTexSrv.GetResource().GetHeight()/2, format, {1, 1, 0, 0});
		Texture2D prepare_tex = context.CreateTexture2D(m_inputTexSrv.GetResource().GetWidth(), m_inputTexSrv.GetResource().GetHeight(), format, { 1, 1, 0, 0 });
		prepare_tex._GetResourcePtr()->SetName("DOF prepare tex");
		m_prepare_rtv = context.CreateRtv(prepare_tex, format, rtvDesc);
		m_prepare_rtv.GetResource()._GetResourcePtr()->SetName("DOF prepare RTV");

		//Texture2D depth_tex = context.CreateTexture2D(m_inputTexSrv.GetResource().GetWidth() / 2, m_inputTexSrv.GetResource().GetHeight() / 2, depthFormat, { 1, 1, 0, 0 });
		Texture2D depth_tex = context.CreateTexture2D(m_inputTexSrv.GetResource().GetWidth(), m_inputTexSrv.GetResource().GetHeight(), depthFormat, { 1, 1, 0, 0 });
		depth_tex._GetResourcePtr()->SetName("DOF depth tex");
		m_depth_rtv = context.CreateRtv(depth_tex, depthFormat, rtvDesc);
		m_depth_rtv.GetResource()._GetResourcePtr()->SetName("DOF depth RTV");
	}
}


} // namespace inl::gxeng::nodes
