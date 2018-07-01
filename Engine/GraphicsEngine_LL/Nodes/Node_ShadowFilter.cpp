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
	Vec2 direction;
	float lightSize;
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

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, inputBindParamDesc },{ samplerDesc });
	}


	if (!m_minfilterPSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.vs = true;
		shaderParts.ps = true;

		m_minfilterShader = context.CreateShader("ShadowMinfilter", shaderParts, "");

		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
		};

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

	//do minfiltering
	for(int c = 0; c < m_filterHorizontal_rtv.size(); ++c)
	{
		uniformsCBData.lightSize = 3.0;
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


}


void ShadowFilter::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatBlur = eFormat::R16_FLOAT;

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
			64,
			64,
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
			m_csmMinfilter_srv.push_back(context.CreateSrv(m_csmTexSrv.GetResource(), FormatDepthToColor(m_csmTexSrv.GetFormat()), srvDesc));
		}

		//create minfilter srvs
		{ //csm
			gxapi::SrvTextureCubeArray cubeSrvDesc;
			cubeSrvDesc.indexOfFirst2DTex = 0;
			cubeSrvDesc.mipLevelClamping = 0;
			cubeSrvDesc.mostDetailedMip = 0;
			cubeSrvDesc.numMipLevels = 1;
			cubeSrvDesc.numCubes = 1;
			m_cubeMinfilter_srv.push_back(context.CreateSrv(m_cubeShadowTexSrv.GetResource(), FormatDepthToColor(m_cubeShadowTexSrv.GetFormat()), cubeSrvDesc));
		}
	}
}


} // namespace inl::gxeng::nodes
