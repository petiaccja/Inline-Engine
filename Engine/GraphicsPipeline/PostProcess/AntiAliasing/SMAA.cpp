#include "SMAA.hpp"

#include "../../Debug/DebugDrawManager.hpp"
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>



namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(SMAA)


struct Uniforms {
	Vec4 pixelSize;
};


SMAA::SMAA() {
	this->GetInput<0>().Set({});
	this->GetInput<1>().Set({});
	this->GetInput<2>().Set({});
}


void SMAA::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void SMAA::Reset() {
	m_inputTexSrv = TextureView2D();

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
}

const std::string& SMAA::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"colorTex",
		"areaImage",
		"searchImage",
		"neighborHoodBlendingRTV",
	};
	return names[index];
}

const std::string& SMAA::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"smaaOutput"
	};
	return names[index];
}

void SMAA::Setup(SetupContext& context) {
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	Texture2D inputTex = this->GetInput<0>().Get();
	m_inputTexSrv = context.CreateSrv(inputTex, inputTex.GetFormat(), srvDesc);

	auto& target = this->GetInput<3>().Get();
	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_neighborhoodBlendingRTV = context.CreateRtv(target, target.GetFormat(), rtvDesc);


	auto areaImage = this->GetInput<1>().Get();
	auto searchImage = this->GetInput<2>().Get();

	if (areaImage == nullptr) {
		throw InvalidArgumentException("Adjál rendes texturát!");
		if (!areaImage->GetSrv()) {
			throw InvalidArgumentException("Given texture was empty.");
		}
	}

	if (searchImage == nullptr) {
		throw InvalidArgumentException("Adjál rendes texturát!");
		if (!searchImage->GetSrv()) {
			throw InvalidArgumentException("Given texture was empty.");
		}
	}


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

		BindParameterDesc areaBindParamDesc;
		m_areaTexBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		areaBindParamDesc.parameter = m_areaTexBindParam;
		areaBindParamDesc.constantSize = 0;
		areaBindParamDesc.relativeAccessFrequency = 0;
		areaBindParamDesc.relativeChangeFrequency = 0;
		areaBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc searchBindParamDesc;
		m_searchTexBindParam = BindParameter(eBindParameterType::TEXTURE, 2);
		searchBindParamDesc.parameter = m_searchTexBindParam;
		searchBindParamDesc.constantSize = 0;
		searchBindParamDesc.relativeAccessFrequency = 0;
		searchBindParamDesc.relativeChangeFrequency = 0;
		searchBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc blendBindParamDesc;
		m_blendTexBindParam = BindParameter(eBindParameterType::TEXTURE, 3);
		blendBindParamDesc.parameter = m_blendTexBindParam;
		blendBindParamDesc.constantSize = 0;
		blendBindParamDesc.relativeAccessFrequency = 0;
		blendBindParamDesc.relativeChangeFrequency = 0;
		blendBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc2;
		samplerDesc2.shaderRegister = 1;
		samplerDesc2.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc2.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc2.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc2.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc2.mipLevelBias = 0.f;
		samplerDesc2.registerSpace = 0;
		samplerDesc2.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, sampBindParamDesc2, inputBindParamDesc, areaBindParamDesc, searchBindParamDesc, blendBindParamDesc }, { samplerDesc, samplerDesc2 });
	}

	if (!m_edgeDetectionPSO || !m_blendingWeightsPSO || !m_neighborhoodBlendingPSO) {
		InitRenderTarget(context);

		{
			ShaderParts shaderParts;
			shaderParts.vs = true;
			shaderParts.ps = true;

			m_edgeDetectionShader = context.CreateShader("SMAAEdgeDetection", shaderParts, "");

			std::vector<gxapi::InputElementDesc> inputElementDesc = {
				gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
				gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
			};

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_edgeDetectionShader.vs;
			psoDesc.ps = m_edgeDetectionShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_edgeDetectionRTV.GetResource().GetFormat();

			m_edgeDetectionPSO.reset(context.CreatePSO(psoDesc));
		}

		{
			ShaderParts shaderParts;
			shaderParts.vs = true;
			shaderParts.ps = true;

			m_blendingWeightsShader = context.CreateShader("SMAABlendingWeights", shaderParts, "");

			std::vector<gxapi::InputElementDesc> inputElementDesc = {
				gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
				gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
			};

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_blendingWeightsShader.vs;
			psoDesc.ps = m_blendingWeightsShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_blendingWeightsRTV.GetResource().GetFormat();

			m_blendingWeightsPSO.reset(context.CreatePSO(psoDesc));
		}

		{
			ShaderParts shaderParts;
			shaderParts.vs = true;
			shaderParts.ps = true;

			m_neighborhoodBlendingShader = context.CreateShader("SMAANeighborhoodBlending", shaderParts, "");

			std::vector<gxapi::InputElementDesc> inputElementDesc = {
				gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
				gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 12)
			};

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_neighborhoodBlendingShader.vs;
			psoDesc.ps = m_neighborhoodBlendingShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;
			psoDesc.renderTargetFormats[0] = m_neighborhoodBlendingRTV.GetResource().GetFormat();

			m_neighborhoodBlendingPSO.reset(context.CreatePSO(psoDesc));
		}
	}

	this->GetOutput<0>().Set(m_neighborhoodBlendingRTV.GetResource());
	//this->GetOutput<0>().Set(m_blendingWeightsRTV.GetResource());
	//this->GetOutput<0>().Set(m_edgeDetectionRTV.GetResource());
}


void SMAA::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Bright Lum pass volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	*/

	uniformsCBData.pixelSize = Vec4(1.0f / Vec2(m_edgeDetectionRTV.GetResource().GetWidth(), m_edgeDetectionRTV.GetResource().GetHeight()), Vec2(m_edgeDetectionRTV.GetResource().GetWidth(), m_edgeDetectionRTV.GetResource().GetHeight()));

	gxapi::Rectangle rect{ 0, (int)m_edgeDetectionRTV.GetResource().GetHeight(), 0, (int)m_edgeDetectionRTV.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;


	{ //edge detection
		commandList.SetResourceState(m_edgeDetectionRTV.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_inputTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_edgeDetectionRTV;
		commandList.SetRenderTargets(1, &pRTV, 0);

		commandList.ClearRenderTarget(m_edgeDetectionRTV, gxapi::ColorRGBA(0, 0, 0, 0));

		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetPipelineState(m_edgeDetectionPSO.get());
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
		commandList.BindGraphics(m_inputTexBindParam, m_inputTexSrv);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}

	{ //blending weights
		auto areaImage = this->GetInput<1>().Get();
		auto searchImage = this->GetInput<2>().Get();

		commandList.SetResourceState(m_blendingWeightsRTV.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_edgeDetectionSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(areaImage->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(searchImage->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_blendingWeightsRTV;
		commandList.SetRenderTargets(1, &pRTV, 0);

		commandList.ClearRenderTarget(m_blendingWeightsRTV, gxapi::ColorRGBA(0, 0, 0, 0));

		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetPipelineState(m_blendingWeightsPSO.get());
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
		commandList.BindGraphics(m_inputTexBindParam, m_edgeDetectionSRV);
		commandList.BindGraphics(m_areaTexBindParam, areaImage->GetSrv());
		commandList.BindGraphics(m_searchTexBindParam, searchImage->GetSrv());
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}

	{ //neighborhood blending
		commandList.SetResourceState(m_neighborhoodBlendingRTV.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_blendingWeightsSRV.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_neighborhoodBlendingRTV;
		commandList.SetRenderTargets(1, &pRTV, 0);

		commandList.SetScissorRects(1, &rect);
		commandList.SetViewports(1, &viewport);

		commandList.SetPipelineState(m_neighborhoodBlendingPSO.get());
		commandList.SetGraphicsBinder(&m_binder);
		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
		commandList.BindGraphics(m_inputTexBindParam, m_inputTexSrv);
		commandList.BindGraphics(m_blendTexBindParam, m_blendingWeightsSRV);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}
}


void SMAA::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto format = eFormat::R8G8B8A8_UNORM;

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
			format
		};
		TextureUsage usage{
			true, true, false, false
		};

		Texture2D edgeDetectionTex = context.CreateTexture2D(desc, usage);
		edgeDetectionTex.SetName("SMAA edge detection tex");
		m_edgeDetectionRTV = context.CreateRtv(edgeDetectionTex, format, rtvDesc);

		m_edgeDetectionSRV = context.CreateSrv(edgeDetectionTex, format, srvDesc);


		Texture2D blendingWeightsTex = context.CreateTexture2D(desc, usage);
		blendingWeightsTex.SetName("SMAA blending weights tex");
		m_blendingWeightsRTV = context.CreateRtv(blendingWeightsTex, format, rtvDesc);

		m_blendingWeightsSRV = context.CreateSrv(blendingWeightsTex, format, srvDesc);


		//Texture2D neighborhoodBlendingTex = context.CreateTexture2D(desc, usage);
		//neighborhoodBlendingTex.SetName("SMAA neighborhood blending tex");
		//m_neighborhoodBlendingRTV = context.CreateRtv(neighborhoodBlendingTex, format, rtvDesc);

		//m_neighborhoodBlendingSRV = context.CreateSrv(neighborhoodBlendingTex, format, srvDesc);
	}
}


} // namespace inl::gxeng::nodes
