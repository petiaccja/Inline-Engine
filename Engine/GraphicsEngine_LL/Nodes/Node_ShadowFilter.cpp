#include "Node_ShadowFilter.hpp"

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
	Mat44 invV;
	Vec4 farPlaneData0, farPlaneData1;
	float lightSize, nearPlane, farPlane, dummy;
	Vec4 vsLightPos;
	Vec2 direction;
};


ShadowFilter::ShadowFilter() {
	this->GetInput<0>().Set({});
}


void ShadowFilter::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void ShadowFilter::Reset() {
	m_csmTexSrv = TextureView2D();
	m_cubeShadowTexSrv = TextureViewCube();

	GetInput<0>().Clear();
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

		gxapi::StaticSamplerDesc csmSamplerDesc;
		csmSamplerDesc.shaderRegister = 500;
		csmSamplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		csmSamplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		csmSamplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		csmSamplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		csmSamplerDesc.mipLevelBias = 0.f;
		csmSamplerDesc.registerSpace = 0;
		csmSamplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, shadowLayersBindParamDesc, penumbraLayersBindParamDesc, csmSampBindParamDesc, cubeMinfilterBindParamDesc, inputBindParamDesc, cubeShadowBindParamDesc, csmMinfilterBindParamDesc, csmBindParamDesc, shadowMxBindParamDesc, csmSplitsBindParamDesc, lightMvpBindParamDesc },{ samplerDesc, csmSamplerDesc });
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
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_minfilterShader.vs;
			psoDesc.ps = m_minfilterShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_filterHorizontal_rtv[0].GetResource().GetFormat();

			m_minfilterPSO.reset(context.CreatePSO(psoDesc));
		}

		{ //penumbra pso
			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_penumbraShader.vs;
			psoDesc.ps = m_penumbraShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 2;
			psoDesc.renderTargetFormats[0] = m_shadowLayers_rtv.GetResource().GetFormat();
			psoDesc.renderTargetFormats[1] = m_penumbraLayers_rtv.GetResource().GetFormat();

			m_penumbraPSO.reset(context.CreatePSO(psoDesc));
		}

		{ //blur pso
			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder->GetRootSignature();
			psoDesc.vs = m_blurShader.vs;
			psoDesc.ps = m_blurShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_blurLayersFirstPass_rtv.GetResource().GetFormat();

			m_blurPSO.reset(context.CreatePSO(psoDesc));
		}
	}

	this->GetOutput<0>().Set(m_filterVertical_rtv[0].GetResource());
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
	for(int c = 0; c < m_filterHorizontal_rtv.size(); ++c)
	{
		uniformsCBData.lightSize = 1.0;
		{ //horizontal blur
			commandList.SetResourceState(m_filterHorizontal_rtv[c].GetResource(), gxapi::eResourceState::RENDER_TARGET);

			RenderTargetView2D* pRTV = &m_filterHorizontal_rtv[c];
			commandList.SetRenderTargets(1, &pRTV, 0);

			gxapi::Rectangle rect{ 0, (int)m_filterHorizontal_rtv[c].GetResource().GetHeight(), 0, (int)m_filterHorizontal_rtv[c].GetResource().GetWidth() };
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
			commandList.SetGraphicsBinder(&m_binder.value());
			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
			commandList.BindGraphics(m_inputTexBindParam, m_inputTex_srv[c]);

			uniformsCBData.direction = Vec2(0, 1);
			commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
			commandList.DrawInstanced(4);
		}

		{ //vertical blur
			commandList.SetResourceState(m_filterVertical_rtv[c].GetResource(), gxapi::eResourceState::RENDER_TARGET);
			commandList.SetResourceState(m_filterHorizontal_rtv[c].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

			RenderTargetView2D* pRTV = &m_filterVertical_rtv[c];
			commandList.SetRenderTargets(1, &pRTV, 0);

			gxapi::Rectangle rect{ 0, (int)m_filterVertical_rtv[c].GetResource().GetHeight(), 0, (int)m_filterVertical_rtv[c].GetResource().GetWidth() };
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
			commandList.SetGraphicsBinder(&m_binder.value());
			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
			commandList.BindGraphics(m_inputTexBindParam, m_filterHorizontal_srv[c]);

			uniformsCBData.direction = Vec2(1, 0);
			commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
			commandList.DrawInstanced(4);
		}
	}

	{ //layer generation pass
		commandList.SetResourceState(m_penumbraLayers_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_shadowLayers_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_csmMinfilter_srv[0].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_cubeMinfilter_srv[0].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV[2] = { &m_shadowLayers_rtv, &m_penumbraLayers_rtv };
		commandList.SetRenderTargets(2, pRTV, 0);

		gxapi::Rectangle rect{ 0, (int)m_penumbraLayers_rtv.GetResource().GetHeight(), 0, (int)m_penumbraLayers_rtv.GetResource().GetWidth() };
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
		commandList.SetGraphicsBinder(&m_binder.value());
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
		commandList.BindGraphics(m_inputTexBindParam, m_depthTexSrv);
		commandList.BindGraphics(m_cubeShadowTexBindParam, m_cubeShadowTexSrv);
		commandList.BindGraphics(m_csmTexBindParam, m_csmTexSrv);
		commandList.BindGraphics(m_shadowMxTexBindParam, m_shadowMxTexSrv);
		commandList.BindGraphics(m_csmSplitsTexBindParam, m_csmSplitsTexSrv);
		commandList.BindGraphics(m_lightMvpTexBindParam, m_lightMvpTexSrv);
		commandList.BindGraphics(m_csmMinfilterTexBindParam, m_csmMinfilter_srv[0]);
		commandList.BindGraphics(m_cubeMinfilterTexBindParam, m_cubeMinfilter_srv[0]);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}

	{ //blur layers pass
		//first pass
		commandList.SetResourceState(m_blurLayersFirstPass_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_csmMinfilter_srv[0].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_cubeMinfilter_srv[0].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_penumbraLayers_rtv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_shadowLayers_rtv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_blurLayersFirstPass_rtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		gxapi::Rectangle rect{ 0, (int)m_blurLayersFirstPass_rtv.GetResource().GetHeight(), 0, (int)m_blurLayersFirstPass_rtv.GetResource().GetWidth() };
		gxapi::Viewport viewport;
		viewport.width = (float)rect.right;
		viewport.height = (float)rect.bottom;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		uniformsCBData.direction = Vec2(0.89442719082100156952748325334158, 0.44721359585778655717526397765935) * 1.11803398875;

		commandList.SetPipelineState(m_blurPSO.get());
		commandList.SetGraphicsBinder(&m_binder.value());
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
		commandList.BindGraphics(m_inputTexBindParam, m_depthTexSrv);
		commandList.BindGraphics(m_cubeShadowTexBindParam, m_cubeShadowTexSrv);
		commandList.BindGraphics(m_csmTexBindParam, m_csmTexSrv);
		commandList.BindGraphics(m_shadowMxTexBindParam, m_shadowMxTexSrv);
		commandList.BindGraphics(m_csmSplitsTexBindParam, m_csmSplitsTexSrv);
		commandList.BindGraphics(m_lightMvpTexBindParam, m_lightMvpTexSrv);
		commandList.BindGraphics(m_csmMinfilterTexBindParam, m_csmMinfilter_srv[0]);
		commandList.BindGraphics(m_cubeMinfilterTexBindParam, m_cubeMinfilter_srv[0]);
		commandList.BindGraphics(m_shadowLayersTexBindParam, m_shadowLayers_srv);
		commandList.BindGraphics(m_penumbraLayersTexBindParam, m_penumbraLayers_srv);

		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);

		//second pass
		commandList.SetResourceState(m_blurLayersSecondPass_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_blurLayersFirstPass_rtv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		pRTV = &m_blurLayersSecondPass_rtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		uniformsCBData.direction = Vec2(-0.44721359585778655717526397765935, 0.89442719082100156952748325334158) * 1.11803398875;
		
		commandList.BindGraphics(m_shadowLayersTexBindParam, m_blurLayersFirstPass_srv);
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

		m_filterHorizontal_rtv.reserve(numRtvs);
		m_filterHorizontal_srv.reserve(numRtvs);
		m_filterVertical_rtv.reserve(numRtvs);

		//csm minfilter tex
		desc.arraySize = 4;
		Texture2D csmMinfilter_tex = context.CreateTexture2D(desc, { true, true, false, false });
		csmMinfilter_tex.SetName("Shadow csm minfilter tex ");

		//TODO for each point light
		//cube minfilter tex
		desc.arraySize = 6;
		Texture2D cubeMinfilter_tex = context.CreateTexture2D(desc, { true, true, false, false });
		cubeMinfilter_tex.SetName("Shadow cube minfilter tex ");

		//horizontal is generic, vertical goes into special textures
		for (int c = 0; c < numRtvs; ++c)
		{
			desc.arraySize = 1;
			Texture2D blurHorizontal_tex = context.CreateTexture2D(desc, { true, true, false, false });
			blurHorizontal_tex.SetName("Shadow minfilter horizontal blur tex " + std::to_string(c));
			m_filterHorizontal_rtv.push_back( context.CreateRtv(blurHorizontal_tex, formatBlur, rtvDesc) );
			m_filterHorizontal_srv.push_back( context.CreateSrv(blurHorizontal_tex, formatBlur, srvDesc) );
		}

		//csm
		rtvDesc.activeArraySize = 1;
		for (int c = 0; c < 4; ++c)
		{
			rtvDesc.firstArrayElement = c;
			m_filterVertical_rtv.push_back(context.CreateRtv(csmMinfilter_tex, formatBlur, rtvDesc));
		}

		//cube
		{ //TODO for each point light
			for (int c = 0; c < 6; ++c)
			{
				rtvDesc.firstArrayElement = c;
				m_filterVertical_rtv.push_back(context.CreateRtv(cubeMinfilter_tex, formatBlur, rtvDesc));
			}
		}


		//create input tex srvs
		m_inputTex_srv.reserve(numRtvs);

		{ //csm
			for (int c = 0; c < 4; ++c)
			{
				srvDesc.firstArrayElement = c;
				m_inputTex_srv.push_back(context.CreateSrv(m_csmTexSrv.GetResource(), FormatDepthToColor(m_csmTexSrv.GetFormat()), srvDesc));
			}
		}

		{ //TODO for each point light
			for (int c = 0; c < 6; ++c)
			{
				srvDesc.firstArrayElement = c;
				m_inputTex_srv.push_back(context.CreateSrv(m_cubeShadowTexSrv.GetResource(), FormatDepthToColor(m_cubeShadowTexSrv.GetFormat()), srvDesc));
			}
		}

		//create minfilter srvs
		m_csmMinfilter_srv.reserve(1);
		m_cubeMinfilter_srv.reserve(1);

		{ //csm
			srvDesc.activeArraySize = 4;
			srvDesc.firstArrayElement = 0;
			m_csmMinfilter_srv.push_back(context.CreateSrv(csmMinfilter_tex, csmMinfilter_tex.GetFormat(), srvDesc));
		}

		//create minfilter srvs
		{ //csm
			gxapi::SrvTextureCubeArray cubeSrvDesc;
			cubeSrvDesc.indexOfFirst2DTex = 0;
			cubeSrvDesc.mipLevelClamping = 0;
			cubeSrvDesc.mostDetailedMip = 0;
			cubeSrvDesc.numMipLevels = 1;
			cubeSrvDesc.numCubes = 1;
			m_cubeMinfilter_srv.push_back(context.CreateSrv(cubeMinfilter_tex, cubeMinfilter_tex.GetFormat(), cubeSrvDesc));
		}

		//create penumbra rtvs
		desc.arraySize = 1;
		desc.width = m_depthTexSrv.GetResource().GetWidth();
		desc.height = m_depthTexSrv.GetResource().GetHeight();
		desc.format = formatShadowLayers;
		Texture2D shadowLayers_tex = context.CreateTexture2D(desc, { true, true, false, false });
		shadowLayers_tex.SetName("Shadow layers tex ");
		desc.format = formatPenumbraLayers;
		Texture2D penumbraLayers_tex = context.CreateTexture2D(desc, { true, true, false, false });
		penumbraLayers_tex.SetName("Penumbra layers tex ");
		desc.format = formatShadowLayers;
		Texture2D blurLayersFirstPass_tex = context.CreateTexture2D(desc, { true, true, false, false });
		blurLayersFirstPass_tex.SetName("Blur layers fist pass tex ");
		Texture2D blurLayersSecondPass_tex = context.CreateTexture2D(desc, { true, true, false, false });
		blurLayersSecondPass_tex.SetName("Blur layers second pass tex ");

		rtvDesc.activeArraySize = 1;
		rtvDesc.firstArrayElement = 0;
		m_shadowLayers_rtv = context.CreateRtv(shadowLayers_tex, shadowLayers_tex.GetFormat(), rtvDesc);
		m_penumbraLayers_rtv = context.CreateRtv(penumbraLayers_tex, penumbraLayers_tex.GetFormat(), rtvDesc);
		m_blurLayersFirstPass_rtv = context.CreateRtv(blurLayersFirstPass_tex, blurLayersFirstPass_tex.GetFormat(), rtvDesc);
		m_blurLayersSecondPass_rtv = context.CreateRtv(blurLayersSecondPass_tex, blurLayersSecondPass_tex.GetFormat(), rtvDesc);

		srvDesc.activeArraySize = 1;
		srvDesc.firstArrayElement = 0;
		m_shadowLayers_srv = context.CreateSrv(shadowLayers_tex, shadowLayers_tex.GetFormat(), srvDesc);
		m_penumbraLayers_srv = context.CreateSrv(penumbraLayers_tex, penumbraLayers_tex.GetFormat(), srvDesc);
		m_blurLayersFirstPass_srv = context.CreateSrv(blurLayersFirstPass_tex, blurLayersFirstPass_tex.GetFormat(), srvDesc);
	}
}


} // namespace inl::gxeng::nodes
