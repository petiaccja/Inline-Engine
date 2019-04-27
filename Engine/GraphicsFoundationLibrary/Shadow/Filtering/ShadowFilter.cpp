#include "ShadowFilter.hpp"

#include "../../Debug/DebugDrawManager.hpp"
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/PerspectiveCamera.hpp>

namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(ShadowFilter)


struct Uniforms {
	Mat44 invV;
	Vec4 farPlaneData0, farPlaneData1;
	float lightSize, nearPlane, farPlane, dummy;
	Vec4 vsLightPos;
	Vec2 direction;
};


ShadowFilter::ShadowFilter() {
	this->GetInput<0>().Set({});
}


void ShadowFilter::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void ShadowFilter::Reset() {
	m_csmTexSrv = TextureView2D();
	m_cubeShadowTexSrv = TextureViewCube();

	GetInput<0>().Clear();
}

const std::string& ShadowFilter::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"csmTex",
		"shadowMxTex",
		"csmSplitsTex",
		"lightMvpTex",
		"cubeShadowTex",
		"depthTex",
		"camera"
	};
	return names[index];
}

const std::string& ShadowFilter::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"layeredShadowMap"
	};
	return names[index];
}

void ShadowFilter::Setup(SetupContext& context) {
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 4;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	Texture2D csmTex = this->GetInput<0>().Get();
	m_csmTexSrv = context.CreateSrv(csmTex, FormatDepthToColor(csmTex.GetFormat()), srvDesc);

	srvDesc.activeArraySize = 1;

	Texture2D shadowMxTex = this->GetInput<1>().Get();
	m_shadowMxTexSrv = context.CreateSrv(shadowMxTex, shadowMxTex.GetFormat(), srvDesc);

	Texture2D csmSplitsTex = this->GetInput<2>().Get();
	m_csmSplitsTexSrv = context.CreateSrv(csmSplitsTex, csmSplitsTex.GetFormat(), srvDesc);

	Texture2D lightMvpTex = this->GetInput<3>().Get();
	m_lightMvpTexSrv = context.CreateSrv(lightMvpTex, lightMvpTex.GetFormat(), srvDesc);

	gxapi::SrvTextureCubeArray cubeSrvDesc;
	cubeSrvDesc.indexOfFirst2DTex = 0;
	cubeSrvDesc.mipLevelClamping = 0;
	cubeSrvDesc.mostDetailedMip = 0;
	cubeSrvDesc.numMipLevels = 1;
	cubeSrvDesc.numCubes = 1;

	Texture2D cubeShadowTex = this->GetInput<4>().Get();
	m_cubeShadowTexSrv = context.CreateSrv(cubeShadowTex, FormatDepthToColor(cubeShadowTex.GetFormat()), cubeSrvDesc);

	Texture2D depthTex = this->GetInput<5>().Get();
	m_depthTexSrv = context.CreateSrv(depthTex, FormatDepthToColor(depthTex.GetFormat()), srvDesc);

	m_camera = this->GetInput<6>().Get();

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

		BindParameterDesc sampLinearBindParamDesc;
		sampLinearBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 1);
		sampLinearBindParamDesc.constantSize = 0;
		sampLinearBindParamDesc.relativeAccessFrequency = 0;
		sampLinearBindParamDesc.relativeChangeFrequency = 0;
		sampLinearBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc inputBindParamDesc;
		m_inputTexBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		inputBindParamDesc.parameter = m_inputTexBindParam;
		inputBindParamDesc.constantSize = 0;
		inputBindParamDesc.relativeAccessFrequency = 0;
		inputBindParamDesc.relativeChangeFrequency = 0;
		inputBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc csmMinfilterBindParamDesc;
		m_csmMinfilterTexBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		csmMinfilterBindParamDesc.parameter = m_csmMinfilterTexBindParam;
		csmMinfilterBindParamDesc.constantSize = 0;
		csmMinfilterBindParamDesc.relativeAccessFrequency = 0;
		csmMinfilterBindParamDesc.relativeChangeFrequency = 0;
		csmMinfilterBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc cubeMinfilterBindParamDesc;
		m_cubeMinfilterTexBindParam = BindParameter(eBindParameterType::TEXTURE, 2);
		cubeMinfilterBindParamDesc.parameter = m_cubeMinfilterTexBindParam;
		cubeMinfilterBindParamDesc.constantSize = 0;
		cubeMinfilterBindParamDesc.relativeAccessFrequency = 0;
		cubeMinfilterBindParamDesc.relativeChangeFrequency = 0;
		cubeMinfilterBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc penumbraLayersBindParamDesc;
		m_penumbraLayersTexBindParam = BindParameter(eBindParameterType::TEXTURE, 3);
		penumbraLayersBindParamDesc.parameter = m_penumbraLayersTexBindParam;
		penumbraLayersBindParamDesc.constantSize = 0;
		penumbraLayersBindParamDesc.relativeAccessFrequency = 0;
		penumbraLayersBindParamDesc.relativeChangeFrequency = 0;
		penumbraLayersBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc shadowLayersBindParamDesc;
		m_shadowLayersTexBindParam = BindParameter(eBindParameterType::TEXTURE, 4);
		shadowLayersBindParamDesc.parameter = m_shadowLayersTexBindParam;
		shadowLayersBindParamDesc.constantSize = 0;
		shadowLayersBindParamDesc.relativeAccessFrequency = 0;
		shadowLayersBindParamDesc.relativeChangeFrequency = 0;
		shadowLayersBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc cubeShadowBindParamDesc;
		m_cubeShadowTexBindParam = BindParameter(eBindParameterType::TEXTURE, 400);
		cubeShadowBindParamDesc.parameter = m_cubeShadowTexBindParam;
		cubeShadowBindParamDesc.constantSize = 0;
		cubeShadowBindParamDesc.relativeAccessFrequency = 0;
		cubeShadowBindParamDesc.relativeChangeFrequency = 0;
		cubeShadowBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc csmSampBindParamDesc;
		csmSampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 500);
		csmSampBindParamDesc.constantSize = 0;
		csmSampBindParamDesc.relativeAccessFrequency = 0;
		csmSampBindParamDesc.relativeChangeFrequency = 0;
		csmSampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc csmBindParamDesc;
		m_csmTexBindParam = BindParameter(eBindParameterType::TEXTURE, 500);
		csmBindParamDesc.parameter = m_csmTexBindParam;
		csmBindParamDesc.constantSize = 0;
		csmBindParamDesc.relativeAccessFrequency = 0;
		csmBindParamDesc.relativeChangeFrequency = 0;
		csmBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc shadowMxBindParamDesc;
		m_shadowMxTexBindParam = BindParameter(eBindParameterType::TEXTURE, 501);
		shadowMxBindParamDesc.parameter = m_shadowMxTexBindParam;
		shadowMxBindParamDesc.constantSize = 0;
		shadowMxBindParamDesc.relativeAccessFrequency = 0;
		shadowMxBindParamDesc.relativeChangeFrequency = 0;
		shadowMxBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc csmSplitsBindParamDesc;
		m_csmSplitsTexBindParam = BindParameter(eBindParameterType::TEXTURE, 502);
		csmSplitsBindParamDesc.parameter = m_csmSplitsTexBindParam;
		csmSplitsBindParamDesc.constantSize = 0;
		csmSplitsBindParamDesc.relativeAccessFrequency = 0;
		csmSplitsBindParamDesc.relativeChangeFrequency = 0;
		csmSplitsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc lightMvpBindParamDesc;
		m_lightMvpTexBindParam = BindParameter(eBindParameterType::TEXTURE, 503);
		lightMvpBindParamDesc.parameter = m_lightMvpTexBindParam;
		lightMvpBindParamDesc.constantSize = 0;
		lightMvpBindParamDesc.relativeAccessFrequency = 0;
		lightMvpBindParamDesc.relativeChangeFrequency = 0;
		lightMvpBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerLinearDesc;
		samplerLinearDesc.shaderRegister = 1;
		samplerLinearDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_LINEAR_MIP_POINT;
		samplerLinearDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerLinearDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerLinearDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerLinearDesc.mipLevelBias = 0.f;
		samplerLinearDesc.registerSpace = 0;
		samplerLinearDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;


		gxapi::StaticSamplerDesc csmSamplerDesc;
		csmSamplerDesc.shaderRegister = 500;
		csmSamplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		csmSamplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		csmSamplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		csmSamplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		csmSamplerDesc.mipLevelBias = 0.f;
		csmSamplerDesc.registerSpace = 0;
		csmSamplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, shadowLayersBindParamDesc, penumbraLayersBindParamDesc, csmSampBindParamDesc, cubeMinfilterBindParamDesc, inputBindParamDesc, cubeShadowBindParamDesc, csmMinfilterBindParamDesc, csmBindParamDesc, shadowMxBindParamDesc, csmSplitsBindParamDesc, lightMvpBindParamDesc }, { samplerDesc, samplerLinearDesc, csmSamplerDesc });
	}


	if (!m_minfilterPSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_minfilterShader = context.CreateShader("ShadowMinfilter", shaderParts, "");
		m_penumbraShader = context.CreateShader("ShadowPenumbra", shaderParts, "");
		m_blurShader = context.CreateShader("ShadowBlur", shaderParts, "");

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
		};

		{ //minfilter pso
			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_minfilterShader.vs;
			psoDesc.ps = m_minfilterShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_filterHorizontalRtv[0].GetResource().GetFormat();

			m_minfilterPSO.reset(context.CreatePSO(psoDesc));
		}

		{ //penumbra pso
			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_penumbraShader.vs;
			psoDesc.ps = m_penumbraShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 2;
			psoDesc.renderTargetFormats[0] = m_shadowLayersRtv.GetResource().GetFormat();
			psoDesc.renderTargetFormats[1] = m_penumbraLayersRtv.GetResource().GetFormat();

			m_penumbraPSO.reset(context.CreatePSO(psoDesc));
		}

		{ //blur pso
			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_blurShader.vs;
			psoDesc.ps = m_blurShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_blurLayersFirstPassRtv.GetResource().GetFormat();

			m_blurPSO.reset(context.CreatePSO(psoDesc));
		}
	}

	this->GetOutput<0>().Set(m_blurLayersSecondPassRtv.GetResource());
}


void ShadowFilter::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Bright Lum pass volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	*/

	commandList.SetResourceState(m_csmTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_cubeShadowTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_csmTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_shadowMxTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_csmSplitsTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_lightMvpTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	commandList.SetResourceState(m_cubeShadowTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	uniformsCBData.nearPlane = m_camera->GetNearPlane();
	uniformsCBData.farPlane = m_camera->GetFarPlane();

	//far ndc corners
	Vec4 ndcCorners[] =
		{
			Vec4(-1.f, -1.f, 1.f, 1.f),
			Vec4(1.f, 1.f, 1.f, 1.f),
		};

	Mat44 p = m_camera->GetProjectionMatrix();
	Mat44 invP = p.Inverse();

	//convert to world space frustum corners
	ndcCorners[0] = ndcCorners[0] * invP;
	ndcCorners[1] = ndcCorners[1] * invP;
	ndcCorners[0] /= ndcCorners[0].w;
	ndcCorners[1] /= ndcCorners[1].w;

	uniformsCBData.farPlaneData0 = Vec4(ndcCorners[0].xyz, ndcCorners[1].x);
	uniformsCBData.farPlaneData1 = Vec4(ndcCorners[1].y, ndcCorners[1].z, 0.0f, 0.0f);

	uniformsCBData.vsLightPos = Vec4(Vec3(0, 0, 1), 1.0f) * m_camera->GetViewMatrix();

	uniformsCBData.invV = m_camera->GetViewMatrix().Inverse();

	//do minfiltering
	for (int c = 0; c < m_filterHorizontalRtv.size(); ++c) {
		uniformsCBData.lightSize = 1.0;
		{ //horizontal blur
			commandList.SetResourceState(m_filterHorizontalRtv[c].GetResource(), gxapi::eResourceState::RENDER_TARGET);

			RenderTargetView2D* pRTV = &m_filterHorizontalRtv[c];
			commandList.SetRenderTargets(1, &pRTV, 0);

			gxapi::Rectangle rect{ 0, (int)m_filterHorizontalRtv[c].GetResource().GetHeight(), 0, (int)m_filterHorizontalRtv[c].GetResource().GetWidth() };
			gxapi::Viewport viewport;
			viewport.width = (float)rect.right;
			viewport.height = (float)rect.bottom;
			viewport.topLeftX = 0;
			viewport.topLeftY = 0;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			commandList.SetScissorRects(1, &rect);
			commandList.SetViewports(1, &viewport);

			commandList.SetPipelineState(m_minfilterPSO.get());
			commandList.SetGraphicsBinder(&m_binder);
			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
			commandList.BindGraphics(m_inputTexBindParam, m_inputTexSrv[c]);

			uniformsCBData.direction = Vec2(0, 1);
			commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
			commandList.DrawInstanced(4);
		}

		{ //vertical blur
			commandList.SetResourceState(m_filterVerticalRtv[c].GetResource(), gxapi::eResourceState::RENDER_TARGET);
			commandList.SetResourceState(m_filterHorizontalRtv[c].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

			RenderTargetView2D* pRTV = &m_filterVerticalRtv[c];
			commandList.SetRenderTargets(1, &pRTV, 0);

			gxapi::Rectangle rect{ 0, (int)m_filterVerticalRtv[c].GetResource().GetHeight(), 0, (int)m_filterVerticalRtv[c].GetResource().GetWidth() };
			gxapi::Viewport viewport;
			viewport.width = (float)rect.right;
			viewport.height = (float)rect.bottom;
			viewport.topLeftX = 0;
			viewport.topLeftY = 0;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			commandList.SetScissorRects(1, &rect);
			commandList.SetViewports(1, &viewport);

			commandList.SetPipelineState(m_minfilterPSO.get());
			commandList.SetGraphicsBinder(&m_binder);
			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
			commandList.BindGraphics(m_inputTexBindParam, m_filterHorizontalSrv[c]);

			uniformsCBData.direction = Vec2(1, 0);
			commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
			commandList.DrawInstanced(4);
		}
	}

	{ //layer generation pass
		commandList.SetResourceState(m_penumbraLayersRtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_shadowLayersRtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_csmMinfilterSrv[0].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_cubeMinfilterSrv[0].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV[2] = { &m_shadowLayersRtv, &m_penumbraLayersRtv };
		commandList.SetRenderTargets(2, pRTV, 0);

		gxapi::Rectangle rect{ 0, (int)m_penumbraLayersRtv.GetResource().GetHeight(), 0, (int)m_penumbraLayersRtv.GetResource().GetWidth() };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetPipelineState(m_penumbraPSO.get());
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
		commandList.BindGraphics(m_inputTexBindParam, m_depthTexSrv);
		commandList.BindGraphics(m_cubeShadowTexBindParam, m_cubeShadowTexSrv);
		commandList.BindGraphics(m_csmTexBindParam, m_csmTexSrv);
		commandList.BindGraphics(m_shadowMxTexBindParam, m_shadowMxTexSrv);
		commandList.BindGraphics(m_csmSplitsTexBindParam, m_csmSplitsTexSrv);
		commandList.BindGraphics(m_lightMvpTexBindParam, m_lightMvpTexSrv);
		commandList.BindGraphics(m_csmMinfilterTexBindParam, m_csmMinfilterSrv[0]);
		commandList.BindGraphics(m_cubeMinfilterTexBindParam, m_cubeMinfilterSrv[0]);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}

	{ //blur layers pass
		//first pass
		commandList.SetResourceState(m_blurLayersFirstPassRtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_csmMinfilterSrv[0].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_cubeMinfilterSrv[0].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_penumbraLayersRtv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_shadowLayersRtv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_blurLayersFirstPassRtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		gxapi::Rectangle rect{ 0, (int)m_blurLayersFirstPassRtv.GetResource().GetHeight(), 0, (int)m_blurLayersFirstPassRtv.GetResource().GetWidth() };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		uniformsCBData.direction = Vec2(0.89442719082100156952748325334158f, 0.44721359585778655717526397765935f) * 1.11803398875f;

		commandList.SetPipelineState(m_blurPSO.get());
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
		commandList.BindGraphics(m_inputTexBindParam, m_depthTexSrv);
		commandList.BindGraphics(m_cubeShadowTexBindParam, m_cubeShadowTexSrv);
		commandList.BindGraphics(m_csmTexBindParam, m_csmTexSrv);
		commandList.BindGraphics(m_shadowMxTexBindParam, m_shadowMxTexSrv);
		commandList.BindGraphics(m_csmSplitsTexBindParam, m_csmSplitsTexSrv);
		commandList.BindGraphics(m_lightMvpTexBindParam, m_lightMvpTexSrv);
		commandList.BindGraphics(m_csmMinfilterTexBindParam, m_csmMinfilterSrv[0]);
		commandList.BindGraphics(m_cubeMinfilterTexBindParam, m_cubeMinfilterSrv[0]);
		commandList.BindGraphics(m_shadowLayersTexBindParam, m_shadowLayersSrv);
		commandList.BindGraphics(m_penumbraLayersTexBindParam, m_penumbraLayersSrv);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);

		//second pass
		commandList.SetResourceState(m_blurLayersSecondPassRtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_blurLayersFirstPassRtv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		pRTV = &m_blurLayersSecondPassRtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		uniformsCBData.direction = Vec2(-0.44721359585778655717526397765935, 0.89442719082100156952748325334158) * 1.11803398875;

		commandList.BindGraphics(m_shadowLayersTexBindParam, m_blurLayersFirstPassSrv);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.DrawInstanced(4);
	}
}


void ShadowFilter::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatBlur = eFormat::R16_FLOAT;
		auto formatShadowLayers = eFormat::R8G8B8A8_UNORM;
		auto formatPenumbraLayers = eFormat::R16G16B16A16_FLOAT;

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
			128,
			128,
			formatBlur,
			1
		};

		//create minfilter rtvs and srvs
		int numRtvs = 6 + 4; //cube + csm

		m_filterHorizontalRtv.reserve(numRtvs);
		m_filterHorizontalSrv.reserve(numRtvs);
		m_filterVerticalRtv.reserve(numRtvs);

		//csm minfilter tex
		desc.arraySize = 4;
		Texture2D csmMinfilterTex = context.CreateTexture2D(desc, { true, true, false, false });
		csmMinfilterTex.SetName("Shadow csm minfilter tex ");

		//TODO for each point light
		//cube minfilter tex
		desc.arraySize = 6;
		Texture2D cubeMinfilterTex = context.CreateTexture2D(desc, { true, true, false, false });
		cubeMinfilterTex.SetName("Shadow cube minfilter tex ");

		//horizontal is generic, vertical goes into special textures
		for (int c = 0; c < numRtvs; ++c) {
			desc.arraySize = 1;
			Texture2D blurHorizontalTex = context.CreateTexture2D(desc, { true, true, false, false });
			blurHorizontalTex.SetName("Shadow minfilter horizontal blur tex " + std::to_string(c));
			m_filterHorizontalRtv.push_back(context.CreateRtv(blurHorizontalTex, formatBlur, rtvDesc));
			m_filterHorizontalSrv.push_back(context.CreateSrv(blurHorizontalTex, formatBlur, srvDesc));
		}

		//csm
		rtvDesc.activeArraySize = 1;
		for (int c = 0; c < 4; ++c) {
			rtvDesc.firstArrayElement = c;
			m_filterVerticalRtv.push_back(context.CreateRtv(csmMinfilterTex, formatBlur, rtvDesc));
		}

		//cube
		{ //TODO for each point light
			for (int c = 0; c < 6; ++c) {
				rtvDesc.firstArrayElement = c;
				m_filterVerticalRtv.push_back(context.CreateRtv(cubeMinfilterTex, formatBlur, rtvDesc));
			}
		}


		//create input tex srvs
		m_inputTexSrv.reserve(numRtvs);

		{ //csm
			for (int c = 0; c < 4; ++c) {
				srvDesc.firstArrayElement = c;
				m_inputTexSrv.push_back(context.CreateSrv(m_csmTexSrv.GetResource(), FormatDepthToColor(m_csmTexSrv.GetFormat()), srvDesc));
			}
		}

		{ //TODO for each point light
			for (int c = 0; c < 6; ++c) {
				srvDesc.firstArrayElement = c;
				m_inputTexSrv.push_back(context.CreateSrv(m_cubeShadowTexSrv.GetResource(), FormatDepthToColor(m_cubeShadowTexSrv.GetFormat()), srvDesc));
			}
		}

		//create minfilter srvs
		m_csmMinfilterSrv.reserve(1);
		m_cubeMinfilterSrv.reserve(1);

		{ //csm
			srvDesc.activeArraySize = 4;
			srvDesc.firstArrayElement = 0;
			m_csmMinfilterSrv.push_back(context.CreateSrv(csmMinfilterTex, csmMinfilterTex.GetFormat(), srvDesc));
		}

		//create minfilter srvs
		{ //csm
			gxapi::SrvTextureCubeArray cubeSrvDesc;
			cubeSrvDesc.indexOfFirst2DTex = 0;
			cubeSrvDesc.mipLevelClamping = 0;
			cubeSrvDesc.mostDetailedMip = 0;
			cubeSrvDesc.numMipLevels = 1;
			cubeSrvDesc.numCubes = 1;
			m_cubeMinfilterSrv.push_back(context.CreateSrv(cubeMinfilterTex, cubeMinfilterTex.GetFormat(), cubeSrvDesc));
		}

		//create penumbra rtvs
		desc.arraySize = 1;
		desc.width = m_depthTexSrv.GetResource().GetWidth();
		desc.height = m_depthTexSrv.GetResource().GetHeight();
		desc.format = formatShadowLayers;
		Texture2D shadowLayersTex = context.CreateTexture2D(desc, { true, true, false, false });
		shadowLayersTex.SetName("Shadow layers tex ");
		desc.format = formatPenumbraLayers;
		Texture2D penumbraLayersTex = context.CreateTexture2D(desc, { true, true, false, false });
		penumbraLayersTex.SetName("Penumbra layers tex ");
		desc.format = formatShadowLayers;
		Texture2D blurLayersFirstPassTex = context.CreateTexture2D(desc, { true, true, false, false });
		blurLayersFirstPassTex.SetName("Blur layers fist pass tex ");
		Texture2D blurLayersSecondPassTex = context.CreateTexture2D(desc, { true, true, false, false });
		blurLayersSecondPassTex.SetName("Blur layers second pass tex ");

		rtvDesc.activeArraySize = 1;
		rtvDesc.firstArrayElement = 0;
		m_shadowLayersRtv = context.CreateRtv(shadowLayersTex, shadowLayersTex.GetFormat(), rtvDesc);
		m_penumbraLayersRtv = context.CreateRtv(penumbraLayersTex, penumbraLayersTex.GetFormat(), rtvDesc);
		m_blurLayersFirstPassRtv = context.CreateRtv(blurLayersFirstPassTex, blurLayersFirstPassTex.GetFormat(), rtvDesc);
		m_blurLayersSecondPassRtv = context.CreateRtv(blurLayersSecondPassTex, blurLayersSecondPassTex.GetFormat(), rtvDesc);

		srvDesc.activeArraySize = 1;
		srvDesc.firstArrayElement = 0;
		m_shadowLayersSrv = context.CreateSrv(shadowLayersTex, shadowLayersTex.GetFormat(), srvDesc);
		m_penumbraLayersSrv = context.CreateSrv(penumbraLayersTex, penumbraLayersTex.GetFormat(), srvDesc);
		m_blurLayersFirstPassSrv = context.CreateSrv(blurLayersFirstPassTex, blurLayersFirstPassTex.GetFormat(), srvDesc);
	}
}


} // namespace inl::gxeng::nodes
