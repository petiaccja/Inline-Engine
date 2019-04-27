#include "ScreenSpaceAmbientOcclusion.hpp"

#include "../../Debug/DebugDrawManager.hpp"
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>



namespace inl::gxeng::nodes {

INL_REGISTER_GRAPHICS_NODE(ScreenSpaceAmbientOcclusion)



struct Uniforms {
	Mat44_Packed invVP, oldVP;
	Vec4_Packed farPlaneData0, farPlaneData1;
	float nearPlane, farPlane, wsRadius, scaleFactor;
	float temporalIndex;
};


static int temporalIndex = 0;

ScreenSpaceAmbientOcclusion::ScreenSpaceAmbientOcclusion() {
	this->GetInput<0>().Set({});
}


void ScreenSpaceAmbientOcclusion::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void ScreenSpaceAmbientOcclusion::Reset() {
	m_depthTexSrv = TextureView2D();

	GetInput<0>().Clear();
}

const std::string& ScreenSpaceAmbientOcclusion::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"depthTex",
		"camera",
		"velocityNormalTex"
	};
	return names[index];
}

const std::string& ScreenSpaceAmbientOcclusion::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"ssaoOutput"
	};
	return names[index];
}

void ScreenSpaceAmbientOcclusion::Setup(SetupContext& context) {
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	Texture2D depthTex = this->GetInput<0>().Get();
	m_depthTexSrv = context.CreateSrv(depthTex, FormatDepthToColor(depthTex.GetFormat()), srvDesc);

	m_camera = this->GetInput<1>().Get();

	//TODO input 2 normal tex

	if (!m_binder) {
		BindParameterDesc uniformsBindParamDesc;
		m_uniformsBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
		uniformsBindParamDesc.parameter = m_uniformsBindParam;
		uniformsBindParamDesc.constantSize = sizeof(Uniforms);
		uniformsBindParamDesc.relativeAccessFrequency = 0;
		uniformsBindParamDesc.relativeChangeFrequency = 0;
		uniformsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc0;
		sampBindParamDesc0.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
		sampBindParamDesc0.constantSize = 0;
		sampBindParamDesc0.relativeAccessFrequency = 0;
		sampBindParamDesc0.relativeChangeFrequency = 0;
		sampBindParamDesc0.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc sampBindParamDesc1;
		sampBindParamDesc1.parameter = BindParameter(eBindParameterType::SAMPLER, 1);
		sampBindParamDesc1.constantSize = 0;
		sampBindParamDesc1.relativeAccessFrequency = 0;
		sampBindParamDesc1.relativeChangeFrequency = 0;
		sampBindParamDesc1.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc dethBindParamDesc;
		m_depthTexBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		dethBindParamDesc.parameter = m_depthTexBindParam;
		dethBindParamDesc.constantSize = 0;
		dethBindParamDesc.relativeAccessFrequency = 0;
		dethBindParamDesc.relativeChangeFrequency = 0;
		dethBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc inputBindParamDesc;
		m_inputTexBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		inputBindParamDesc.parameter = m_inputTexBindParam;
		inputBindParamDesc.constantSize = 0;
		inputBindParamDesc.relativeAccessFrequency = 0;
		inputBindParamDesc.relativeChangeFrequency = 0;
		inputBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc temporalBindParamDesc;
		m_temporalTexBindParam = BindParameter(eBindParameterType::TEXTURE, 2);
		temporalBindParamDesc.parameter = m_temporalTexBindParam;
		temporalBindParamDesc.constantSize = 0;
		temporalBindParamDesc.relativeAccessFrequency = 0;
		temporalBindParamDesc.relativeChangeFrequency = 0;
		temporalBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc0;
		samplerDesc0.shaderRegister = 0;
		samplerDesc0.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc0.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc0.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc0.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc0.mipLevelBias = 0.f;
		samplerDesc0.registerSpace = 0;
		samplerDesc0.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc1;
		samplerDesc1.shaderRegister = 1;
		samplerDesc1.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samplerDesc1.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc1.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc1.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc1.mipLevelBias = 0.f;
		samplerDesc1.registerSpace = 0;
		samplerDesc1.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc0, sampBindParamDesc1, dethBindParamDesc, inputBindParamDesc, temporalBindParamDesc }, { samplerDesc0, samplerDesc1 });
	}


	if (!m_PSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
		};

		{
			m_shader = context.CreateShader("ScreenSpaceAmbientOcclusion", shaderParts, "");

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
			psoDesc.renderTargetFormats[0] = m_ssaoRtv.GetResource().GetFormat();

			m_PSO.reset(context.CreatePSO(psoDesc));
		}

		{
			m_horizontalShader = context.CreateShader("SsaoBilateralHorizontalBlur", shaderParts, "");

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_horizontalShader.vs;
			psoDesc.ps = m_horizontalShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_blurHorizontalRtv.GetResource().GetFormat();

			m_horizontalPSO.reset(context.CreatePSO(psoDesc));
		}

		{
			m_verticalShader = context.CreateShader("SsaoBilateralVerticalBlur", shaderParts, "");

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_verticalShader.vs;
			psoDesc.ps = m_verticalShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;

			psoDesc.renderTargetFormats[0] = m_blurVertical0Rtv.GetResource().GetFormat();
			m_vertical0PSO.reset(context.CreatePSO(psoDesc));

			psoDesc.renderTargetFormats[0] = m_blurVertical1Rtv.GetResource().GetFormat();
			m_vertical1PSO.reset(context.CreatePSO(psoDesc));
		}
	}

	this->GetOutput<0>().Set(temporalIndex % 2 ? m_blurVertical0Rtv.GetResource() : m_blurVertical1Rtv.GetResource());
	//this->GetOutput<0>().Set(m_ssao_rtv.GetResource());
	//this->GetOutput<0>().Set(m_blur_horizontal_rtv.GetResource());
}


void ScreenSpaceAmbientOcclusion::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Bright Lum pass volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	*/


	uniformsCBData.temporalIndex = (float)temporalIndex;
	temporalIndex = (temporalIndex + 1) % 6;

	Mat44 v = m_camera->GetViewMatrix();
	Mat44 p = m_camera->GetProjectionMatrix();
	Mat44 vp = v * p;
	Mat44 invP = p.Inverse();
	Mat44 invVP = vp.Inverse();
	uniformsCBData.invVP = invVP;
	uniformsCBData.oldVP = m_prevVP;
	uniformsCBData.nearPlane = m_camera->GetNearPlane();
	uniformsCBData.farPlane = m_camera->GetFarPlane();

	uniformsCBData.wsRadius = 0.5f;
	uniformsCBData.scaleFactor = 0.5f * (m_depthTexSrv.GetResource().GetHeight() / (2.0f * p(0, 0)));

	//far ndc corners
	Vec4 ndcCorners[] =
		{
			Vec4(-1.f, -1.f, 1.f, 1.f),
			Vec4(1.f, 1.f, 1.f, 1.f),
		};

	//convert to world space frustum corners
	ndcCorners[0] = ndcCorners[0] * invP;
	ndcCorners[1] = ndcCorners[1] * invP;
	ndcCorners[0] /= ndcCorners[0].w;
	ndcCorners[1] /= ndcCorners[1].w;

	uniformsCBData.farPlaneData0 = Vec4(ndcCorners[0].xyz, ndcCorners[1].x);
	uniformsCBData.farPlaneData1 = Vec4(ndcCorners[1].y, ndcCorners[1].z, 0.0f, 0.0f);

	{ //SSAO pass
		commandList.SetResourceState(m_ssaoRtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_ssaoRtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		gxapi::Rectangle rect{ 0, (int)m_ssaoRtv.GetResource().GetHeight(), 0, (int)m_ssaoRtv.GetResource().GetWidth() };
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

		commandList.BindGraphics(m_depthTexBindParam, m_depthTexSrv);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}


	{ //Bilateral horizontal blur pass
		commandList.SetResourceState(m_blurHorizontalRtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_ssaoSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_blurHorizontalRtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		gxapi::Rectangle rect{ 0, (int)m_blurHorizontalRtv.GetResource().GetHeight(), 0, (int)m_blurHorizontalRtv.GetResource().GetWidth() };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetPipelineState(m_horizontalPSO.get());
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

		commandList.BindGraphics(m_depthTexBindParam, m_depthTexSrv);
		commandList.BindGraphics(m_inputTexBindParam, m_ssaoSrv);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}

	{ //Bilateral vertical blur pass
		auto writeRtv = temporalIndex % 2 ? m_blurVertical0Rtv : m_blurVertical1Rtv;
		auto pso = temporalIndex % 2 ? &m_vertical0PSO : &m_vertical1PSO;
		auto readSrv = temporalIndex % 2 ? m_blurVertical1Srv : m_blurVertical0Srv;

		commandList.SetResourceState(writeRtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_blurHorizontalSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(readSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &writeRtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		gxapi::Rectangle rect{ 0, (int)writeRtv.GetResource().GetHeight(), 0, (int)writeRtv.GetResource().GetWidth() };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetPipelineState(pso->get());
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

		commandList.BindGraphics(m_depthTexBindParam, m_depthTexSrv);
		commandList.BindGraphics(m_temporalTexBindParam, readSrv);
		commandList.BindGraphics(m_inputTexBindParam, m_blurHorizontalSrv);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}

	m_prevVP = vp;
}


void ScreenSpaceAmbientOcclusion::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatSSAO = eFormat::R8G8B8A8_UNORM;

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
			m_depthTexSrv.GetResource().GetWidth(),
			m_depthTexSrv.GetResource().GetHeight(),
			formatSSAO
		};

		Texture2D ssaoTex = context.CreateTexture2D(desc, { true, true, false, false });
		ssaoTex.SetName("Screen space ambient occlusion tex");
		m_ssaoRtv = context.CreateRtv(ssaoTex, formatSSAO, rtvDesc);
		m_ssaoSrv = context.CreateSrv(ssaoTex, formatSSAO, srvDesc);

		Texture2D blurVertical1Tex = context.CreateTexture2D(desc, { true, true, false, false });
		blurVertical1Tex.SetName("Screen space ambient occlusion vertical blur tex");
		m_blurVertical1Rtv = context.CreateRtv(blurVertical1Tex, formatSSAO, rtvDesc);
		m_blurVertical1Srv = context.CreateSrv(blurVertical1Tex, formatSSAO, srvDesc);

		Texture2D blurVertical0Tex = context.CreateTexture2D(desc, { true, true, false, false });
		blurVertical0Tex.SetName("Screen space ambient occlusion vertical blur tex");
		m_blurVertical0Rtv = context.CreateRtv(blurVertical0Tex, formatSSAO, rtvDesc);
		m_blurVertical0Srv = context.CreateSrv(blurVertical0Tex, formatSSAO, srvDesc);

		Texture2D blurHorizontalTex = context.CreateTexture2D(desc, { true, true, false, false });
		blurHorizontalTex.SetName("Screen space ambient occlusion horizontal blur tex");
		m_blurHorizontalRtv = context.CreateRtv(blurHorizontalTex, formatSSAO, rtvDesc);
		m_blurHorizontalSrv = context.CreateSrv(blurHorizontalTex, formatSSAO, srvDesc);
	}
}


} // namespace inl::gxeng::nodes
