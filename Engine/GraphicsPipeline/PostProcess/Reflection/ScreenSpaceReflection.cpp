#include "ScreenSpaceReflection.hpp"

#include "../../Debug/DebugDrawManager.hpp"
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>

namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(ScreenSpaceReflection)


struct Uniforms {
	Mat44_Packed projSS;
	Mat44_Packed v;
	Mat44_Packed invV;
	Vec4_Packed vsCamPos;
	float nearPlane, farPlane, stride, jitter;
	Vec4_Packed farPlaneData0, farPlaneData1;
	Vec2_Packed direction;
	float maxDistance;
};


ScreenSpaceReflection::ScreenSpaceReflection() {
	this->GetInput<0>().Set({});
}


void ScreenSpaceReflection::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void ScreenSpaceReflection::Reset() {
	m_inputTexSrv = TextureView2D();

	GetInput<0>().Clear();
}

const std::string& ScreenSpaceReflection::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"colorTex",
		"depthTex",
		"camera",
		"velocityNormalTex"
	};
	return names[index];
}

const std::string& ScreenSpaceReflection::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"screenSpaceReflectionTex"
	};
	return names[index];
}

void ScreenSpaceReflection::Setup(SetupContext& context) {
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;

	Texture2D inputTex = this->GetInput<0>().Get();
	m_inputTexSrv = context.CreateSrv(inputTex, inputTex.GetFormat(), srvDesc);


	Texture2D depthTex = this->GetInput<1>().Get();
	m_depthTexSrv = context.CreateSrv(depthTex, FormatDepthToColor(depthTex.GetFormat()), srvDesc);

	m_camera = this->GetInput<2>().Get();

	//TODO input 3 normal tex

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

		BindParameterDesc inputBindParamDesc;
		m_inputTexBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		inputBindParamDesc.parameter = m_inputTexBindParam;
		inputBindParamDesc.constantSize = 0;
		inputBindParamDesc.relativeAccessFrequency = 0;
		inputBindParamDesc.relativeChangeFrequency = 0;
		inputBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc dethBindParamDesc;
		m_depthTexBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		dethBindParamDesc.parameter = m_depthTexBindParam;
		dethBindParamDesc.constantSize = 0;
		dethBindParamDesc.relativeAccessFrequency = 0;
		dethBindParamDesc.relativeChangeFrequency = 0;
		dethBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

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

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc0, sampBindParamDesc1, inputBindParamDesc, dethBindParamDesc }, { samplerDesc0, samplerDesc1 });
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
			m_shader = context.CreateShader("ScreenSpaceReflection", shaderParts, "");

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
			psoDesc.renderTargetFormats[0] = m_ssrRtv.GetResource().GetFormat();

			m_PSO.reset(context.CreatePSO(psoDesc));
		}

		{
			m_downsampleShader = context.CreateShader("SsrDownsample", shaderParts, "");

			gxapi::GraphicsPipelineStateDesc psoDesc;
			psoDesc.inputLayout.elements = inputElementDesc.data();
			psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
			psoDesc.rootSignature = m_binder.GetRootSignature();
			psoDesc.vs = m_downsampleShader.vs;
			psoDesc.ps = m_downsampleShader.ps;
			psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
			psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

			psoDesc.depthStencilState.enableDepthTest = false;
			psoDesc.depthStencilState.enableDepthStencilWrite = false;
			psoDesc.depthStencilState.enableStencilTest = false;
			psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;

			psoDesc.numRenderTargets = 1;

			unsigned numMips = m_ssrRtv.GetResource().GetNumMiplevels();

			m_downsamplePSO.resize(numMips);

			for (unsigned c = 0; c < numMips; ++c) {
				psoDesc.renderTargetFormats[0] = m_inputRtv[c].GetResource().GetFormat();
				m_downsamplePSO[c].reset(context.CreatePSO(psoDesc));
			}
		}

		{
			m_blurShader = context.CreateShader("SsrBlur", shaderParts, "");

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

			unsigned numMips = m_ssrRtv.GetResource().GetNumMiplevels();

			m_blurHorizontalPSO.resize(numMips);
			m_blurVerticalPSO.resize(numMips);

			for (unsigned c = 0; c < numMips; ++c) {
				psoDesc.renderTargetFormats[0] = m_blurRtv[c].GetResource().GetFormat();
				m_blurHorizontalPSO[c].reset(context.CreatePSO(psoDesc));

				psoDesc.renderTargetFormats[0] = m_inputRtv[c].GetResource().GetFormat();
				m_blurVerticalPSO[c].reset(context.CreatePSO(psoDesc));
			}
		}
	}

	this->GetOutput<0>().Set(m_ssrRtv.GetResource());
}


void ScreenSpaceReflection::Execute(RenderContext& context) {
	GraphicsCommandList& commandList = context.AsGraphics();

	Uniforms uniformsCBData;

	bool peti = false;
	if (peti) {
		return;
	}

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	//create single-frame only cb
	/*gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Bright Lum pass volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));
	*/


	Mat44 v = m_camera->GetViewMatrix();
	Mat44 p = m_camera->GetProjectionMatrix();
	Mat44 vp = v * p;
	Mat44 invVP = vp.Inverse();
	Mat44 invP = p.Inverse();
	Mat44 mulHalf = {
		0.5, 0, 0, 0,
		0, -0.5, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	Mat44 addHalf = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0.5, 0.5, 0, 1
	};
	Mat44 mulSS = {
		m_inputTexSrv.GetResource().GetWidth(), 0, 0, 0,
		0, m_inputTexSrv.GetResource().GetHeight(), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	uniformsCBData.projSS = p * mulHalf * addHalf * mulSS;
	uniformsCBData.nearPlane = m_camera->GetNearPlane();
	uniformsCBData.farPlane = m_camera->GetFarPlane();
	uniformsCBData.stride = 1;
	uniformsCBData.jitter = 0;
	uniformsCBData.maxDistance = 1000.0;

	uniformsCBData.vsCamPos = Vec4(m_camera->GetPosition(), 1.0) * v;

	uniformsCBData.v = v;
	uniformsCBData.invV = v.Inverse();

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

	{ //fill mip chain
		unsigned numMips = m_ssrRtv.GetResource().GetNumMiplevels();

		uint64_t w = m_inputTexSrv.GetResource().GetWidth();
		uint32_t h = m_inputTexSrv.GetResource().GetHeight();

		for (unsigned c = 1; c < numMips; ++c) {
			w /= 2;
			h /= 2;

			commandList.SetResourceState(m_inputRtv[c].GetResource(), gxapi::eResourceState::RENDER_TARGET, c);
			commandList.SetResourceState(m_inputSrv[c - 1].GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

			RenderTargetView2D* pRTV = &m_inputRtv[c];
			commandList.SetRenderTargets(1, &pRTV, 0);

			gxapi::Rectangle rect{ 0, (int)w, 0, (int)h };
			gxapi::Viewport viewport;
			viewport.width = (float)rect.right;
			viewport.height = (float)rect.bottom;
			viewport.topLeftX = 0;
			viewport.topLeftY = 0;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			commandList.SetScissorRects(1, &rect);
			commandList.SetViewports(1, &viewport);

			commandList.SetPipelineState(m_downsamplePSO[c].get());
			commandList.SetGraphicsBinder(&m_binder);
			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

			commandList.BindGraphics(m_inputTexBindParam, m_inputSrv[c - 1]);
			commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

			commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
			commandList.DrawInstanced(4);
		}
	}

	{ //trace rays
		commandList.SetResourceState(m_ssrRtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
		commandList.SetResourceState(m_inputTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_depthTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		RenderTargetView2D* pRTV = &m_ssrRtv;
		commandList.SetRenderTargets(1, &pRTV, 0);

		gxapi::Rectangle rect{ 0, (int)m_ssrRtv.GetResource().GetHeight(), 0, (int)m_ssrRtv.GetResource().GetWidth() };
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
		commandList.BindGraphics(m_depthTexBindParam, m_depthTexSrv);
		commandList.BindGraphics(m_uniformsBindParam, &uniformsCBData, sizeof(Uniforms));

		commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLESTRIP);
		commandList.DrawInstanced(4);
	}
}


void ScreenSpaceReflection::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatSSR = eFormat::R16G16B16A16_FLOAT;

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
			formatSSR
		};

		Texture2D ssrTex = context.CreateTexture2D(desc, { true, true, false, false });
		ssrTex.SetName("Screen space reflection tex");
		m_ssrRtv = context.CreateRtv(ssrTex, formatSSR, rtvDesc);


		unsigned numMips = m_ssrRtv.GetResource().GetNumMiplevels();

		Texture2DDesc mipDesc;
		mipDesc.arraySize = 1;
		mipDesc.format = formatSSR;
		mipDesc.width = m_inputTexSrv.GetResource().GetWidth();
		mipDesc.height = m_inputTexSrv.GetResource().GetHeight();
		mipDesc.mipLevels = numMips;
		Texture2D blurTex = context.CreateTexture2D(mipDesc, { true, true, false, false });
		blurTex.SetName("Screen space reflection blur tex");

		for (unsigned c = 0; c < numMips; ++c) {
			gxapi::RtvTexture2DArray rtvMipDesc;
			rtvMipDesc.activeArraySize = 1;
			rtvMipDesc.firstArrayElement = 0;
			rtvMipDesc.firstMipLevel = c;
			rtvMipDesc.planeIndex = 0;
			m_inputRtv.push_back(context.CreateRtv(m_inputTexSrv.GetResource(), m_inputTexSrv.GetFormat(), rtvMipDesc));
			m_blurRtv.push_back(context.CreateRtv(blurTex, blurTex.GetFormat(), rtvMipDesc));

			gxapi::SrvTexture2DArray srvMipDesc;
			srvMipDesc.activeArraySize = 1;
			srvMipDesc.firstArrayElement = 0;
			srvMipDesc.numMipLevels = 1;
			srvMipDesc.mipLevelClamping = 0;
			srvMipDesc.mostDetailedMip = c;
			srvMipDesc.planeIndex = 0;
			m_inputSrv.push_back(context.CreateSrv(m_inputTexSrv.GetResource(), m_inputTexSrv.GetFormat(), srvMipDesc));
			m_blurSrv.push_back(context.CreateSrv(blurTex, blurTex.GetFormat(), srvMipDesc));
		}
	}
}


} // namespace inl::gxeng::nodes
