#include "Node_DrawSky.hpp"

#include "../PerspectiveCamera.hpp"
#include "../GraphicsCommandList.hpp"

#include <array>


namespace inl::gxeng::nodes {



void DrawSky::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}


void DrawSky::Setup(SetupContext & context) {

	//================================================
	// Set inputs, outputs

	auto renderTarget = this->GetInput<0>().Get();
	this->GetInput<1>().Clear();
	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_rtv = context.CreateRtv(renderTarget, renderTarget.GetFormat(), rtvDesc);

	auto depthStencil = this->GetInput<1>().Get();
	this->GetInput<1>().Clear();
	gxapi::DsvTexture2DArray dsvDesc;
	dsvDesc.activeArraySize = 1;
	dsvDesc.firstArrayElement = 0;
	dsvDesc.firstMipLevel = 0;
	m_dsv = context.CreateDsv(depthStencil, depthStencil.GetFormat(), dsvDesc);

	m_camera = this->GetInput<2>().Get();
	this->GetInput<2>().Clear();

	m_sun = this->GetInput<3>().Get();
	this->GetInput<3>().Clear();

	this->GetOutput<0>().Set(renderTarget);


	if (!m_binder.has_value()) {
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
		samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		m_binder = context.CreateBinder({ sunCbBindParamDesc, camCbBindParamDesc, sampBindParamDesc }, { samplerDesc });
	}

	//================================================
	// Create or modify pipeline state and other complementary objects if needed

	if (!m_fsq.HasObject()) {
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
		m_fsq = context.CreateVertexBuffer(vertices.data(), sizeof(float)*vertices.size());
		m_fsqIndices = context.CreateIndexBuffer(indices.data(), sizeof(uint16_t)*indices.size(), indices.size());
	}

	if (!m_shader.ps || !m_shader.vs) {
		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_shader = context.CreateShader("DrawSky", shaderParts, "");
	}

	if (m_colorFormat != renderTarget.GetFormat() || m_depthStencilFormat != depthStencil.GetFormat()) {
		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0)
		};

		m_colorFormat = renderTarget.GetFormat();
		m_depthStencilFormat = renderTarget.GetFormat();

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


void DrawSky::Execute(RenderContext & context) {
	gxeng::GraphicsCommandList& commandList = context.AsGraphics();

	auto* pRTV = &m_rtv;
	commandList.SetResourceState(m_rtv.GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_dsv.GetResource(), 0, gxapi::eResourceState::DEPTH_READ);
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
	commandList.SetGraphicsBinder(&m_binder.value());
	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
	commandList.SetStencilRef(0); // only allow sky to be rendered to background pixels

	gxeng::VertexBuffer* pVertexBuffer = &m_fsq;
	unsigned vbSize = (unsigned)m_fsq.GetSize();
	unsigned vbStride = 3 * sizeof(float);

	struct Sun {
		mathfu::VectorPacked<float, 4> dir;
		mathfu::VectorPacked<float, 4> color;

	};

	struct Cam {
		mathfu::VectorPacked<float, 4> invViewProj[4];
		mathfu::VectorPacked<float, 4> pos;
	};

	Sun sunCB;
	Cam camCB;

	mathfu::Matrix4x4f viewInvTr = m_camera->GetViewMatrixRH().Inverse().Transpose();
	mathfu::Vector4f sunViewDir = viewInvTr * mathfu::Vector4f(m_sun->GetDirection(), 0.0f);
	mathfu::Vector4f sunColor = mathfu::Vector4f(m_sun->GetColor(), 1.0f);
	mathfu::Matrix4x4f invViewProj = (m_camera->GetProjectionMatrixRH() * m_camera->GetViewMatrixRH()).Inverse();

	sunCB.dir = mathfu::Vector4f(m_sun->GetDirection(), 0.0);
	sunCB.color = sunColor;
	invViewProj.Pack(camCB.invViewProj);

	const PerspectiveCamera* perpectiveCamera = dynamic_cast<const PerspectiveCamera*>(m_camera);
	if (perpectiveCamera == nullptr) {
		throw std::invalid_argument("Sky drawing only works with perspective camera");
	}

	camCB.pos = mathfu::Vector4f(perpectiveCamera->GetPosition(), 1);

	commandList.BindGraphics(m_sunCbBindParam, &sunCB, sizeof(sunCB), 0);
	commandList.BindGraphics(m_camCbBindParam, &camCB, sizeof(camCB), 0);
	commandList.SetVertexBuffers(0, 1, &pVertexBuffer, &vbSize, &vbStride);
	commandList.SetIndexBuffer(&m_fsqIndices, false);
	commandList.DrawIndexedInstanced((unsigned)m_fsqIndices.GetIndexCount());
}

} // namespace inl::gxeng::nodes
