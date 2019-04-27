#include "DrawSky.hpp"

#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/PerspectiveCamera.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>



namespace inl::gxeng::nodes {

INL_REGISTER_GRAPHICS_NODE(DrawSky)


void DrawSky::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void DrawSky::Reset() {
	m_rtv = {};
	m_dsv = {};

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
	GetInput<3>().Clear();
}


void DrawSky::Setup(SetupContext& context) {

	//================================================
	// Set inputs, outputs

	auto renderTarget = this->GetInput<0>().Get();
	auto depthStencil = this->GetInput<1>().Get();
	if (!renderTarget) {
		throw InvalidArgumentException("Render target cannot be null.");
	}
	if (!depthStencil) {
		throw InvalidArgumentException("Depth-stencil cannot be null.");
	}

	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_rtv = context.CreateRtv(renderTarget, renderTarget.GetFormat(), rtvDesc);


	const gxapi::eFormat currDepthStencilFormat = FormatAnyToDepthStencil(depthStencil.GetFormat());
	gxapi::DsvTexture2DArray dsvDesc;
	dsvDesc.activeArraySize = 1;
	dsvDesc.firstArrayElement = 0;
	dsvDesc.firstMipLevel = 0;
	m_dsv = context.CreateDsv(depthStencil, currDepthStencilFormat, dsvDesc);

	this->GetOutput<0>().Set(renderTarget);

	if (!m_binder) {
		BindParameterDesc sunCbBindParamDesc;
		m_sunCbBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
		sunCbBindParamDesc.parameter = m_sunCbBindParam;
		sunCbBindParamDesc.constantSize = sizeof(float) * 4 * 2;
		sunCbBindParamDesc.relativeAccessFrequency = 0;
		sunCbBindParamDesc.relativeChangeFrequency = 0;
		sunCbBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc camCbBindParamDesc;
		m_camCbBindParam = BindParameter(eBindParameterType::CONSTANT, 1);
		camCbBindParamDesc.parameter = m_camCbBindParam;
		camCbBindParamDesc.constantSize = sizeof(float) * 16 + sizeof(float) * 4;
		camCbBindParamDesc.relativeAccessFrequency = 0;
		camCbBindParamDesc.relativeChangeFrequency = 0;
		camCbBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc;
		sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
		sampBindParamDesc.constantSize = 0;
		sampBindParamDesc.relativeAccessFrequency = 0;
		sampBindParamDesc.relativeChangeFrequency = 0;
		sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		m_binder = context.CreateBinder({ sunCbBindParamDesc, camCbBindParamDesc, sampBindParamDesc }, { samplerDesc });
	}

	//================================================
	// Create or modify pipeline state and other complementary objects if needed


	if (!m_shader.ps || !m_shader.vs) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_shader = context.CreateShader("DrawSky", shaderParts, "");
	}

	if (m_colorFormat != renderTarget.GetFormat() || m_depthStencilFormat != currDepthStencilFormat) {
		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0)
		};

		m_colorFormat = renderTarget.GetFormat();
		m_depthStencilFormat = currDepthStencilFormat;

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
		psoDesc.depthStencilState.enableStencilTest = true;
		psoDesc.depthStencilState.stencilReadMask = ~uint8_t(0);
		psoDesc.depthStencilState.stencilWriteMask = 0;
		psoDesc.depthStencilState.ccwFace.stencilFunc = gxapi::eComparisonFunction::EQUAL;
		psoDesc.depthStencilState.ccwFace.stencilOpOnStencilFail = gxapi::eStencilOp::KEEP;
		psoDesc.depthStencilState.ccwFace.stencilOpOnDepthFail = gxapi::eStencilOp::KEEP;
		psoDesc.depthStencilState.ccwFace.stencilOpOnPass = gxapi::eStencilOp::KEEP;
		psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;
		psoDesc.depthStencilFormat = m_depthStencilFormat;

		psoDesc.numRenderTargets = 1;
		psoDesc.renderTargetFormats[0] = m_colorFormat;

		m_PSO.reset(context.CreatePSO(psoDesc));
	}
}


void DrawSky::Execute(RenderContext& context) {
	auto* suns = this->GetInput<3>().Get();

	if (!suns || suns->Size() == 0) {
		return;
	}

	gxeng::GraphicsCommandList& commandList = context.AsGraphics();

	auto* pRTV = &m_rtv;
	commandList.SetResourceState(m_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_dsv.GetResource(), gxapi::eResourceState::DEPTH_WRITE);
	commandList.SetRenderTargets(1, &pRTV, &m_dsv);

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
	commandList.SetStencilRef(0); // only allow sky to be rendered to background pixels


	struct Sun {
		Vec4_Packed dir;
		Vec4_Packed color;
	};

	struct Cam {
		Mat44 invViewProj;
		Vec4_Packed pos;
	};

	Sun sunCB;
	Cam camCB;

	auto* camera = this->GetInput<2>().Get();

	// TODO render all the suns using additive blending
	auto sun = *suns->begin();

	Mat44 viewInv = camera->GetViewMatrix().Inverse();
	Vec4 sunViewDir = Vec4(sun->GetDirection(), 0.0f) * viewInv;
	Vec4 sunColor = Vec4(sun->GetColor(), 1.0f);
	Mat44 invViewProj = (camera->GetViewMatrix() * camera->GetProjectionMatrix()).Inverse();

	sunCB.dir = Vec4(sun->GetDirection(), 0.0);
	sunCB.color = sunColor;
	camCB.invViewProj = invViewProj;

	const PerspectiveCamera* perpectiveCamera = dynamic_cast<const PerspectiveCamera*>(camera);
	if (perpectiveCamera == nullptr) {
		throw InvalidArgumentException("Sky drawing only works with perspective camera");
	}

	camCB.pos = Vec4(perpectiveCamera->GetPosition(), 1);

	commandList.BindGraphics(m_sunCbBindParam, &sunCB, sizeof(sunCB));
	commandList.BindGraphics(m_camCbBindParam, &camCB, sizeof(camCB));
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
	commandList.DrawInstanced(4);
}


const std::string& DrawSky::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Render target",
		"Stencil",
		"Camera",
		"Lights"
	};
	return names[index];
}

const std::string& DrawSky::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"SkyTex",
	};
	return names[index];
}



} // namespace inl::gxeng::nodes
