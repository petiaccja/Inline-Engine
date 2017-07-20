#include "Node_HDRCombine.hpp"

#include "NodeUtility.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../PerspectiveCamera.hpp"
#include "../GraphicsCommandList.hpp"
#include "../EntityCollection.hpp"
#include "AssetLibrary/Image.hpp"

#include "DebugDrawManager.hpp"

#include <array>

namespace inl::gxeng::nodes {

struct Uniforms
{
	Mat44_Packed lensStarMx;
	float exposure, bloom_weight;
};


HDRCombine::HDRCombine() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
	this->GetInput<2>().Set({});
	this->GetInput<3>().Set({});
	this->GetInput<4>().Set({});
	this->GetInput<5>().Set({});
	this->GetInput<6>().Set({});
	this->GetInput<7>().Set({});
}


void HDRCombine::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void HDRCombine::Reset() {
	m_inputTexSrv = TextureView2D();
	m_luminanceTexSrv = TextureView2D();
	m_bloomTexSrv = TextureView2D();
	m_lensFlareTexSrv = TextureView2D();
	m_colorGradingImage = nullptr;
	m_lensFlareDirtImage = nullptr;
	m_lensFlareStarImage = nullptr;
	m_camera = nullptr;

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
	GetInput<3>().Clear();
	GetInput<4>().Clear();
	GetInput<5>().Clear();
	GetInput<6>().Clear();
	GetInput<7>().Clear();
}


void HDRCombine::Setup(SetupContext& context) {
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	Texture2D inputTex = this->GetInput<0>().Get();
	m_inputTexSrv = context.CreateSrv(inputTex, inputTex.GetFormat(), srvDesc);
	m_inputTexSrv.GetResource()._GetResourcePtr()->SetName("HDR Combine input tex SRV");

	Texture2D luminanceTex = this->GetInput<1>().Get();
	m_luminanceTexSrv = context.CreateSrv(luminanceTex, luminanceTex.GetFormat(), srvDesc);
	m_luminanceTexSrv.GetResource()._GetResourcePtr()->SetName("HDR Combine luminance tex SRV");

	Texture2D bloomTex = this->GetInput<2>().Get();
	m_bloomTexSrv = context.CreateSrv(bloomTex, bloomTex.GetFormat(), srvDesc);
	m_bloomTexSrv.GetResource()._GetResourcePtr()->SetName("HDR Combine bloom tex SRV");

	Texture2D lensFlareTex = this->GetInput<3>().Get();
	m_lensFlareTexSrv = context.CreateSrv(lensFlareTex, lensFlareTex.GetFormat(), srvDesc);
	m_lensFlareTexSrv.GetResource()._GetResourcePtr()->SetName("HDR Combine lens flare tex SRV");

	m_colorGradingImage = this->GetInput<4>().Get();
	m_lensFlareDirtImage = this->GetInput<5>().Get();
	m_lensFlareStarImage = this->GetInput<6>().Get();

	m_camera = this->GetInput<7>().Get();

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

		BindParameterDesc inputBindParamDesc;
		m_inputTexBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		inputBindParamDesc.parameter = m_inputTexBindParam;
		inputBindParamDesc.constantSize = 0;
		inputBindParamDesc.relativeAccessFrequency = 0;
		inputBindParamDesc.relativeChangeFrequency = 0;
		inputBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc luminanceBindParamDesc;
		m_luminanceTexBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		luminanceBindParamDesc.parameter = m_luminanceTexBindParam;
		luminanceBindParamDesc.constantSize = 0;
		luminanceBindParamDesc.relativeAccessFrequency = 0;
		luminanceBindParamDesc.relativeChangeFrequency = 0;
		luminanceBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc bloomBindParamDesc;
		m_bloomTexBindParam = BindParameter(eBindParameterType::TEXTURE, 2);
		bloomBindParamDesc.parameter = m_bloomTexBindParam;
		bloomBindParamDesc.constantSize = 0;
		bloomBindParamDesc.relativeAccessFrequency = 0;
		bloomBindParamDesc.relativeChangeFrequency = 0;
		bloomBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc colorGradingBindParamDesc;
		m_colorGradingTexBindParam = BindParameter(eBindParameterType::TEXTURE, 3);
		colorGradingBindParamDesc.parameter = m_colorGradingTexBindParam;
		colorGradingBindParamDesc.constantSize = 0;
		colorGradingBindParamDesc.relativeAccessFrequency = 0;
		colorGradingBindParamDesc.relativeChangeFrequency = 0;
		colorGradingBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc lensFlareBindParamDesc;
		m_lensFlareTexBindParam = BindParameter(eBindParameterType::TEXTURE, 4);
		lensFlareBindParamDesc.parameter = m_lensFlareTexBindParam;
		lensFlareBindParamDesc.constantSize = 0;
		lensFlareBindParamDesc.relativeAccessFrequency = 0;
		lensFlareBindParamDesc.relativeChangeFrequency = 0;
		lensFlareBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc lensFlareDirtBindParamDesc;
		m_lensFlareDirtTexBindParam = BindParameter(eBindParameterType::TEXTURE, 5);
		lensFlareDirtBindParamDesc.parameter = m_lensFlareDirtTexBindParam;
		lensFlareDirtBindParamDesc.constantSize = 0;
		lensFlareDirtBindParamDesc.relativeAccessFrequency = 0;
		lensFlareDirtBindParamDesc.relativeChangeFrequency = 0;
		lensFlareDirtBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc lensFlareStarBindParamDesc;
		m_lensFlareStarTexBindParam = BindParameter(eBindParameterType::TEXTURE, 6);
		lensFlareStarBindParamDesc.parameter = m_lensFlareStarTexBindParam;
		lensFlareStarBindParamDesc.constantSize = 0;
		lensFlareStarBindParamDesc.relativeAccessFrequency = 0;
		lensFlareStarBindParamDesc.relativeChangeFrequency = 0;
		lensFlareStarBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, inputBindParamDesc, luminanceBindParamDesc, bloomBindParamDesc, colorGradingBindParamDesc, lensFlareBindParamDesc, lensFlareDirtBindParamDesc, lensFlareStarBindParamDesc },{ samplerDesc });
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
		m_fsq._GetResourcePtr()->SetName("HDR Combine full screen quad vertex buffer");
		m_fsqIndices = context.CreateIndexBuffer(indices.data(), sizeof(uint16_t)*indices.size(), indices.size());
		m_fsqIndices._GetResourcePtr()->SetName("HDR Combine pass full screen quad index buffer");
	}

	if (!m_PSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_shader = context.CreateShader("HDRCombine", shaderParts, "");

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

		psoDesc.numRenderTargets = 1;
		psoDesc.renderTargetFormats[0] = m_combine_rtv.GetResource().GetFormat();

		m_PSO.reset(context.CreatePSO(psoDesc));
	}

	this->GetOutput<0>().Set(m_combine_rtv.GetResource());
}


void HDRCombine::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	Vec3 up = m_camera->GetUpVector();
	Vec3 view = m_camera->GetLookDirection();
	Vec3 right = Cross(view, up).Normalized();

	float camrot = Dot(-right, Vec3(0, 0, 1)) + Dot(view, Vec3(0, 1, 0));

	Mat44 scale_bias1(
		2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	Mat44 rotation(
		cos(camrot), sin(camrot), 0.0f, 0.0f, 
		-sin(camrot), cos(camrot), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	Mat44 scale_bias2(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.5f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	uniformsCBData.lensStarMx = scale_bias1 * rotation * scale_bias2;

	//TODO get from somewhere
	uniformsCBData.exposure = 0.0f;
	uniformsCBData.bloom_weight = 1.0f;

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb._GetResourcePtr()->SetName("Bright Lum pass volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	cbv.GetResource()._GetResourcePtr()->SetName("Bright Lum pass CBV");*/

	commandList.SetResourceState(m_combine_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_inputTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_luminanceTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_bloomTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_colorGradingLutTexture->GetSrv()->GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_lensFlareTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_lensFlareDirtTexture->GetSrv()->GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_lensFlareStarTexture->GetSrv()->GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	RenderTargetView2D* pRTV = &m_combine_rtv;
	commandList.SetRenderTargets(1, &pRTV, 0);

	gxapi::Rectangle rect{ 0, (int)m_combine_rtv.GetResource().GetHeight(), 0, (int)m_combine_rtv.GetResource().GetWidth() };
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
	commandList.BindGraphics(m_luminanceTexBindParam, m_luminanceTexSrv);
	commandList.BindGraphics(m_bloomTexBindParam, m_bloomTexSrv);
	commandList.BindGraphics(m_colorGradingTexBindParam, *m_colorGradingLutTexture->GetSrv().get());
	commandList.BindGraphics(m_lensFlareTexBindParam, m_lensFlareTexSrv);
	commandList.BindGraphics(m_lensFlareDirtTexBindParam, *m_lensFlareDirtTexture->GetSrv().get());
	commandList.BindGraphics(m_lensFlareStarTexBindParam, *m_lensFlareStarTexture->GetSrv().get());
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


void HDRCombine::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatHDRCombine = eFormat::R8G8B8A8_UNORM;

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

		Texture2D combine_tex = context.CreateTexture2D(m_inputTexSrv.GetResource().GetWidth(), m_inputTexSrv.GetResource().GetHeight(), formatHDRCombine, {1, 1, 0, 0});
		combine_tex._GetResourcePtr()->SetName("HDR Combine tex");
		m_combine_rtv = context.CreateRtv(combine_tex, formatHDRCombine, rtvDesc);
		m_combine_rtv.GetResource()._GetResourcePtr()->SetName("HDR Combine RTV");

		// Create color grading texture
		{
			using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
			inl::asset::Image img("assets\\colorGrading\\default_lut_table.png");

			m_colorGradingLutTexture.reset(m_colorGradingImage);
			m_colorGradingLutTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
			m_colorGradingLutTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), img.GetData(), PixelT::Reader());

			//TODO
			//create cube texture
		}

		{
			using PixelT = Pixel<ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR>;
			inl::asset::Image img("assets\\lensFlare\\lens_dirt.png");

			m_lensFlareDirtTexture.reset(m_lensFlareDirtImage);
			m_lensFlareDirtTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR);
			m_lensFlareDirtTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), img.GetData(), PixelT::Reader());
		}

		{
			using PixelT = Pixel<ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR>;
			inl::asset::Image img("assets\\lensFlare\\lens_star.png");

			m_lensFlareStarTexture.reset(m_lensFlareStarImage);
			m_lensFlareStarTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR);
			m_lensFlareStarTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), img.GetData(), PixelT::Reader());
		}
	}
}


} // namespace inl::gxeng::nodes
