#include "HDRCombine.hpp"

#include "../../Debug/DebugDrawManager.hpp"
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>



namespace inl::gxeng::nodes {

INL_REGISTER_GRAPHICS_NODE(HDRCombine)

struct Uniforms {
	Mat44_Packed lensStarMx;
	float exposure, bloomWeight;
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


void HDRCombine::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void HDRCombine::Reset() {
	m_inputTexSrv = TextureView2D();
	m_luminanceTexSrv = TextureView2D();
	m_bloomTexSrv = TextureView2D();
	m_lensFlareTexSrv = TextureView2D();
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

const std::string& HDRCombine::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"colorTex",
		"luminanceTex",
		"bloomTex",
		"lensFlareTex",
		"colorGradingImage",
		"lensFlareDirtImage",
		"lensFlareStarImage",
		"camera"
	};
	return names[index];
}

const std::string& HDRCombine::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"combinedOutput"
	};
	return names[index];
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


	Texture2D luminanceTex = this->GetInput<1>().Get();
	m_luminanceTexSrv = context.CreateSrv(luminanceTex, luminanceTex.GetFormat(), srvDesc);


	Texture2D bloomTex = this->GetInput<2>().Get();
	m_bloomTexSrv = context.CreateSrv(bloomTex, bloomTex.GetFormat(), srvDesc);


	Texture2D lensFlareTex = this->GetInput<3>().Get();
	m_lensFlareTexSrv = context.CreateSrv(lensFlareTex, lensFlareTex.GetFormat(), srvDesc);


	auto colorGradingImage = this->GetInput<4>().Get();
	auto lensFlareDirtImage = this->GetInput<5>().Get();
	auto lensFlareStarImage = this->GetInput<6>().Get();

	if (colorGradingImage == nullptr) {
		throw InvalidArgumentException("Adjál rendes texturát!");
		if (!colorGradingImage->GetSrv()) {
			throw InvalidArgumentException("Given texture was empty.");
		}
	}

	if (lensFlareDirtImage == nullptr) {
		throw InvalidArgumentException("Adjál rendes texturát!");
		if (!lensFlareDirtImage->GetSrv()) {
			throw InvalidArgumentException("Given texture was empty.");
		}
	}

	if (lensFlareStarImage == nullptr) {
		throw InvalidArgumentException("Adjál rendes texturát!");
		if (!lensFlareStarImage->GetSrv()) {
			throw InvalidArgumentException("Given texture was empty.");
		}
	}

	m_camera = this->GetInput<7>().Get();

	if (!m_binder) {
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

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, inputBindParamDesc, luminanceBindParamDesc, bloomBindParamDesc, colorGradingBindParamDesc, lensFlareBindParamDesc, lensFlareDirtBindParamDesc, lensFlareStarBindParamDesc }, { samplerDesc });
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
		psoDesc.rootSignature = m_binder.GetRootSignature();
		psoDesc.vs = m_shader.vs;
		psoDesc.ps = m_shader.ps;
		psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
		psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

		psoDesc.depthStencilState.enableDepthTest = false;
		psoDesc.depthStencilState.enableDepthStencilWrite = false;
		psoDesc.depthStencilState.enableStencilTest = false;
		psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

		psoDesc.numRenderTargets = 1;
		psoDesc.renderTargetFormats[0] = m_combineRtv.GetResource().GetFormat();

		m_PSO.reset(context.CreatePSO(psoDesc));
	}

	this->GetOutput<0>().Set(m_combineRtv.GetResource());
}


void HDRCombine::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	Vec3 up = m_camera->GetUpVector();
	Vec3 view = m_camera->GetLookDirection();
	Vec3 right = Cross(view, up).Normalized();

	float camrot = Dot(-right, Vec3(0, 0, 1)) + Dot(view, Vec3(0, 1, 0));

	Mat44 scaleBias1(
		2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	Mat44 rotation(
		cos(camrot), sin(camrot), 0.0f, 0.0f,
		-sin(camrot), cos(camrot), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	Mat44 scaleBias2(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.5f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	uniformsCBData.lensStarMx = scaleBias1 * rotation * scaleBias2;

	//TODO get from somewhere
	//each stop higher on exposure value (EV) doubles incoming light by setting iso/aperture/shutter speed higher
	//-3 stop = 0.125x normal
	//-2 stop = 0.25x normal
	//-1 stop = 0.5x normal
	// 0 stop = 1x normal
	// 1 stop = 2x normal
	// 2 stop = 4x normal
	// 3 stop = 8x normal
	float eVstops = 0.0;
	uniformsCBData.exposure = std::pow(2.0f, eVstops);
	uniformsCBData.bloomWeight = 1.0f;

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Bright Lum pass volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	*/

	auto colorGradingImage = this->GetInput<4>().Get();
	auto lensFlareDirtImage = this->GetInput<5>().Get();
	auto lensFlareStarImage = this->GetInput<6>().Get();

	commandList.SetResourceState(m_combineRtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_inputTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_luminanceTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_bloomTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(colorGradingImage->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_lensFlareTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(lensFlareDirtImage->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(lensFlareStarImage->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	RenderTargetView2D* pRTV = &m_combineRtv;
	commandList.SetRenderTargets(1, &pRTV, 0);

	gxapi::Rectangle rect{ 0, (int)m_combineRtv.GetResource().GetHeight(), 0, (int)m_combineRtv.GetResource().GetWidth() };
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

	commandList.SetPipelineState(m_PSO.get());
	commandList.SetGraphicsBinder(&m_binder);
	commandList.BindGraphics(m_inputTexBindParam, m_inputTexSrv);
	commandList.BindGraphics(m_luminanceTexBindParam, m_luminanceTexSrv);
	commandList.BindGraphics(m_bloomTexBindParam, m_bloomTexSrv);
	commandList.BindGraphics(m_colorGradingTexBindParam, colorGradingImage->GetSrv());
	commandList.BindGraphics(m_lensFlareTexBindParam, m_lensFlareTexSrv);
	commandList.BindGraphics(m_lensFlareDirtTexBindParam, lensFlareDirtImage->GetSrv());
	commandList.BindGraphics(m_lensFlareStarTexBindParam, lensFlareStarImage->GetSrv());
	commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
	commandList.DrawInstanced(4);
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

		Texture2DDesc desc{
			m_inputTexSrv.GetResource().GetWidth(),
			m_inputTexSrv.GetResource().GetHeight(),
			formatHDRCombine
		};

		Texture2D combineTex = context.CreateTexture2D(desc, { true, true, false, false });
		combineTex.SetName("HDR Combine tex");
		m_combineRtv = context.CreateRtv(combineTex, formatHDRCombine, rtvDesc);
	}
}


} // namespace inl::gxeng::nodes
