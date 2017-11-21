#include "Node_DOFMain.hpp"

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
	float tileSize;
};


DOFMain::DOFMain() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
	this->GetInput<2>().Set({});
	this->GetInput<3>().Set({});
	this->GetInput<4>().Set({});
	this->GetInput<5>().Set({});
}


void DOFMain::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void DOFMain::Reset() {
	m_inputTexSrv = TextureView2D();
	m_halfDepthTexSrv = TextureView2D();
	m_neighborhoodMaxTexSrv = TextureView2D();
	m_originalTexSrv = TextureView2D();
	m_depthTexSrv = TextureView2D();
	m_camera = nullptr;

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
	GetInput<3>().Clear();
	GetInput<4>().Clear();
	GetInput<5>().Clear();
}


void DOFMain::Setup(SetupContext& context) {
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	Texture2D inputTex = this->GetInput<0>().Get();
	m_inputTexSrv = context.CreateSrv(inputTex, inputTex.GetFormat(), srvDesc);
	

	Texture2D halfDepthTex = this->GetInput<1>().Get();
	m_halfDepthTexSrv = context.CreateSrv(halfDepthTex, FormatDepthToColor(halfDepthTex.GetFormat()), srvDesc);
	

	Texture2D neighborhoodMaxTex = this->GetInput<2>().Get();
	m_neighborhoodMaxTexSrv = context.CreateSrv(neighborhoodMaxTex, neighborhoodMaxTex.GetFormat(), srvDesc);
	

	Texture2D originalTex = this->GetInput<4>().Get();
	m_originalTexSrv = context.CreateSrv(originalTex, originalTex.GetFormat(), srvDesc);
	

	Texture2D depthTex = this->GetInput<5>().Get();
	m_depthTexSrv = context.CreateSrv(depthTex, FormatDepthToColor(depthTex.GetFormat()), srvDesc);
	

	m_camera = this->GetInput<3>().Get();

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

		BindParameterDesc neighborhoodMaxBindParamDesc;
		m_neighborhoodMaxTexBindParam = BindParameter(eBindParameterType::TEXTURE, 2);
		neighborhoodMaxBindParamDesc.parameter = m_neighborhoodMaxTexBindParam;
		neighborhoodMaxBindParamDesc.constantSize = 0;
		neighborhoodMaxBindParamDesc.relativeAccessFrequency = 0;
		neighborhoodMaxBindParamDesc.relativeChangeFrequency = 0;
		neighborhoodMaxBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

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

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, sampBindParamDesc2, inputBindParamDesc, depthBindParamDesc, neighborhoodMaxBindParamDesc }, { samplerDesc, samplerDesc2 });
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
		m_fsq.SetName("DOF full screen quad vertex buffer");
		m_fsqIndices = context.CreateIndexBuffer(indices.data(), sizeof(uint16_t)*indices.size(), indices.size());
		m_fsqIndices.SetName("DOF full screen quad index buffer");
	}

	if (!m_main_PSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
		};

		{
			m_main_shader = context.CreateShader("DOFMain", shaderParts, "");

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_main_shader.vs;
			psoDesc.ps = m_main_shader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_main_rtv.GetResource().GetFormat();

			m_main_PSO.reset(context.CreatePSO(psoDesc));
		}

		{
			m_postfilter_shader = context.CreateShader("DOFPostfilter", shaderParts, "");

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_postfilter_shader.vs;
			psoDesc.ps = m_postfilter_shader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_postfilter_rtv.GetResource().GetFormat();

			m_postfilter_PSO.reset(context.CreatePSO(psoDesc));
		}

		{
			m_upsample_shader = context.CreateShader("DOFUpsample", shaderParts, "");

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_upsample_shader.vs;
			psoDesc.ps = m_upsample_shader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_upsample_rtv.GetResource().GetFormat();

			m_upsample_PSO.reset(context.CreatePSO(psoDesc));
		}
	}

	//this->GetOutput<0>().Set(m_upsample_rtv.GetResource());
	this->GetOutput<0>().Set(m_postfilter_rtv.GetResource());
	//this->GetOutput<0>().Set(m_main_rtv.GetResource());
}


void DOFMain::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Bright Lum pass volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	*/

	uniformsCBData.maxBlurDiameter = 33.0;
	uniformsCBData.tileSize = 20.0;

	commandList.SetResourceState(m_inputTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_halfDepthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_neighborhoodMaxTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	float fnear = m_camera->GetNearPlane();
	float ffar = m_camera->GetFarPlane();

	Vec3 pos = m_camera->GetPosition();

	gxapi::Rectangle rect{ 0, (int)m_main_rtv.GetResource().GetHeight(), 0, (int)m_main_rtv.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	gxeng::VertexBuffer* pVertexBuffer = &m_fsq;
	unsigned vbSize = (unsigned)m_fsq.GetSize();
	unsigned vbStride = 5 * sizeof(float);

	{ //main pass
		commandList.SetResourceState(m_main_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);

		RenderTargetView2D* pRTV = &m_main_rtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetPipelineState(m_main_PSO.get());
		commandList.SetGraphicsBinder(&m_binder.value());
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

		commandList.BindGraphics(m_inputTexBindParam, m_inputTexSrv);
		commandList.BindGraphics(m_depthTexBindParam, m_halfDepthTexSrv);
		commandList.BindGraphics(m_neighborhoodMaxTexBindParam, m_neighborhoodMaxTexSrv);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetResourceState(*pVertexBuffer, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
		commandList.SetResourceState(m_fsqIndices, gxapi::eResourceState::INDEX_BUFFER);
		commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
		commandList.SetIndexBuffer(&m_fsqIndices, false);
		commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
	}

	{ //postfilter
		commandList.SetResourceState(m_postfilter_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_main_srv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_originalTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_postfilter_rtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetPipelineState(m_postfilter_PSO.get());
		commandList.SetGraphicsBinder(&m_binder.value());
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

		commandList.BindGraphics(m_inputTexBindParam, m_main_srv);
		commandList.BindGraphics(m_depthTexBindParam, m_halfDepthTexSrv);
		commandList.BindGraphics(m_neighborhoodMaxTexBindParam, m_originalTexSrv);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetResourceState(*pVertexBuffer, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
		commandList.SetResourceState(m_fsqIndices, gxapi::eResourceState::INDEX_BUFFER);
		commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
		commandList.SetIndexBuffer(&m_fsqIndices, false);
		commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
	}

	{ //upsample
		/*commandList.SetResourceState(m_upsample_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_postfilter_srv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_originalTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_upsample_rtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		gxapi::Rectangle fullrect{ 0, (int)m_upsample_rtv.GetResource().GetHeight(), 0, (int)m_upsample_rtv.GetResource().GetWidth() };
		gxapi::Viewport fullviewport;
		fullviewport.width = (float)fullrect.right;
		fullviewport.height = (float)fullrect.bottom;
		fullviewport.topLeftX = 0;
		fullviewport.topLeftY = 0;
		fullviewport.minDepth = 0.0f;
		fullviewport.maxDepth = 1.0f;

		commandList.SetScissorRects(1, &fullrect);
		commandList.SetViewports(1, &fullviewport);

		commandList.SetPipelineState(m_upsample_PSO.get());
		commandList.SetGraphicsBinder(&m_binder.value());
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

		commandList.BindGraphics(m_inputTexBindParam, m_postfilter_srv);
		commandList.BindGraphics(m_depthTexBindParam, m_depthTexSrv);
		commandList.BindGraphics(m_neighborhoodMaxTexBindParam, m_originalTexSrv);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetResourceState(*pVertexBuffer, gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
		commandList.SetResourceState(m_fsqIndices, gxapi::eResourceState::INDEX_BUFFER);
		commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
		commandList.SetIndexBuffer(&m_fsqIndices, false);
		commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());*/
	}
}


void DOFMain::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto format = eFormat::R16G16B16A16_FLOAT;

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

		Texture2DDesc desc;

		desc = {
			m_inputTexSrv.GetResource().GetWidth(),
			m_inputTexSrv.GetResource().GetHeight(),
			format,
		};

		Texture2D main_tex = context.CreateTexture2D(desc, { true, true, false, false });
		main_tex.SetName("DOF main tex");
		m_main_rtv = context.CreateRtv(main_tex, format, rtvDesc);
		
		m_main_srv = context.CreateSrv(main_tex, format, srvDesc);
		

		Texture2D postfilter_tex = context.CreateTexture2D(desc, { true, true, false, false });
		postfilter_tex.SetName("DOF postfilter tex");
		m_postfilter_rtv = context.CreateRtv(postfilter_tex, format, rtvDesc);
		
		m_postfilter_srv = context.CreateSrv(postfilter_tex, format, srvDesc);
		

		desc = {
			m_originalTexSrv.GetResource().GetWidth(),
			m_originalTexSrv.GetResource().GetHeight(), 
			format
		};

		Texture2D upsample_tex = context.CreateTexture2D(desc, { true, true, false, false });
		upsample_tex.SetName("DOF upsample tex");
		m_upsample_rtv = context.CreateRtv(upsample_tex, format, rtvDesc);
		
	}
}


} // namespace inl::gxeng::nodes
