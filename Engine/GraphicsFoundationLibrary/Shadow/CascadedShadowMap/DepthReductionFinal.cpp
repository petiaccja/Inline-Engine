#include "DepthReductionFinal.hpp"

#include "../../Debug/DebugDrawManager.hpp"
#include <GraphicsEngine_LL/Nodes/NodeUtility.hpp>

#include <GraphicsEngine_LL/AutoRegisterNode.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>



namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(DepthReductionFinal)


struct Uniforms {
	Mat44_Packed invVP;
	Mat44_Packed biasMx, invMv;
	Vec4_Packed camPos, camViewDir, camUpVector;
	Vec4_Packed lightCamPos, lightCamViewDir, lightCamUpVector;
	float camNear, camFar, texSize;
	float dummy;
};


DepthReductionFinal::DepthReductionFinal() {
	this->GetInput<0>().Set({});
}


void DepthReductionFinal::Initialize(EngineContext& context) {
	GraphicsNode::SetTaskSingle(this);
}

void DepthReductionFinal::Reset() {
	m_reductionTexSrv = TextureView2D();
	m_camera = nullptr;
	m_suns = nullptr;

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
}

const std::string& DepthReductionFinal::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"reductionTex",
		"camera",
		"directionalLights"
	};
	return names[index];
}

const std::string& DepthReductionFinal::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"lightMvpTex",
		"shadowMxTex",
		"csmSplitsTex",
		"csmExtentsTex"
	};
	return names[index];
}

void DepthReductionFinal::Setup(SetupContext& context) {
	InitRenderTarget(context);


	Texture2D reductionTex = this->GetInput<0>().Get();

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;
	m_reductionTexSrv = context.CreateSrv(reductionTex, reductionTex.GetFormat(), srvDesc);


	m_camera = this->GetInput<1>().Get();
	auto dirLights = this->GetInput<2>().Get();
	if (dirLights && dirLights->Size() > 0) {
		m_suns = dirLights;
	}

	this->GetOutput<0>().Set(m_lightMvpUav.GetResource());
	this->GetOutput<1>().Set(m_shadowMxUav.GetResource());
	this->GetOutput<2>().Set(m_csmSplitsUav.GetResource());
	this->GetOutput<3>().Set(m_csmExtentsUav.GetResource());

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

		BindParameterDesc reductionBindParamDesc;
		m_reductionBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		reductionBindParamDesc.parameter = m_reductionBindParam;
		reductionBindParamDesc.constantSize = 0;
		reductionBindParamDesc.relativeAccessFrequency = 0;
		reductionBindParamDesc.relativeChangeFrequency = 0;
		reductionBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc outputBindParamDesc0;
		m_outputBindParam0 = BindParameter(eBindParameterType::UNORDERED, 0);
		outputBindParamDesc0.parameter = m_outputBindParam0;
		outputBindParamDesc0.constantSize = 0;
		outputBindParamDesc0.relativeAccessFrequency = 0;
		outputBindParamDesc0.relativeChangeFrequency = 0;
		outputBindParamDesc0.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc outputBindParamDesc1;
		m_outputBindParam1 = BindParameter(eBindParameterType::UNORDERED, 1);
		outputBindParamDesc1.parameter = m_outputBindParam1;
		outputBindParamDesc1.constantSize = 0;
		outputBindParamDesc1.relativeAccessFrequency = 0;
		outputBindParamDesc1.relativeChangeFrequency = 0;
		outputBindParamDesc1.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc outputBindParamDesc2;
		m_outputBindParam2 = BindParameter(eBindParameterType::UNORDERED, 2);
		outputBindParamDesc2.parameter = m_outputBindParam2;
		outputBindParamDesc2.constantSize = 0;
		outputBindParamDesc2.relativeAccessFrequency = 0;
		outputBindParamDesc2.relativeChangeFrequency = 0;
		outputBindParamDesc2.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		BindParameterDesc outputBindParamDesc3;
		m_outputBindParam3 = BindParameter(eBindParameterType::UNORDERED, 3);
		outputBindParamDesc3.parameter = m_outputBindParam3;
		outputBindParamDesc3.constantSize = 0;
		outputBindParamDesc3.relativeAccessFrequency = 0;
		outputBindParamDesc3.relativeChangeFrequency = 0;
		outputBindParamDesc3.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
		samplerDesc.addressU = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::CLAMP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

		m_binder = context.CreateBinder({ uniformsBindParamDesc, sampBindParamDesc, reductionBindParamDesc, outputBindParamDesc0, outputBindParamDesc1, outputBindParamDesc2, outputBindParamDesc3 }, { samplerDesc });
	}

	if (!m_CSO) {
		InitRenderTarget(context);

		ShaderParts shaderParts;
		shaderParts.cs = true;

		m_shader = context.CreateShader("DepthReductionFinal", shaderParts, "");

		gxapi::ComputePipelineStateDesc csoDesc;
		csoDesc.rootSignature = m_binder.GetRootSignature();
		csoDesc.cs = m_shader.cs;

		m_CSO.reset(context.CreatePSO(csoDesc));
	}
}


void DepthReductionFinal::Execute(RenderContext& context) {
	if (!m_suns) {
		return;
	}

	ComputeCommandList& commandList = context.AsCompute();

	Uniforms uniformsCBData;

	//DebugDrawManager::GetInstance().AddSphere(m_camera->GetPosition() + m_camera->GetLookDirection() * 5, 1, 1);

	Mat44 view = m_camera->GetViewMatrix();
	Mat44 projection = m_camera->GetProjectionMatrix();
	Mat44 vp = view * projection;

	uniformsCBData.invVP = vp.Inverse();

	Mat44 biasMatrix(0.5f, 0, 0, 0,
					 0, -0.5f, 0, 0,
					 0, 0, 1.0f, 0,
					 0.5f, 0.5f, 0.0f, 1);
	uniformsCBData.biasMx = biasMatrix;

	uniformsCBData.invMv = view.Inverse();

	const PerspectiveCamera* perpectiveCamera = dynamic_cast<const PerspectiveCamera*>(m_camera);
	if (perpectiveCamera == nullptr) {
		throw InvalidArgumentException("Depth reduction only works with perspective camera");
	}

	Vec4 camPos(perpectiveCamera->GetPosition(), 1.0f);
	Vec4 camViewDir(perpectiveCamera->GetLookDirection().Normalized(), 0.0f);
	Vec4 camUpVector(perpectiveCamera->GetUpVector().Normalized(), 0.0f);

	uniformsCBData.camPos = camPos;
	uniformsCBData.camViewDir = camViewDir;
	uniformsCBData.camUpVector = camUpVector;

	uniformsCBData.camNear = perpectiveCamera->GetNearPlane();
	uniformsCBData.camFar = perpectiveCamera->GetFarPlane();

	assert((*m_suns)->Size() > 0);
	auto sun = *(*m_suns)->begin();
	//TODO get from somewhere
	Vec4 lightCamPos = Vec4(0.f, 0.f, 0.f, 1.f);
	Vec4 lightCamViewDir = Vec4(sun->GetDirection().Normalized(), 0); //Vec4(1, 1, 1, 0).Normalized();
	//printf("%f %f %f\n", light_cam_view_dir.x, light_cam_view_dir.y, light_cam_view_dir.z);
	Vec4 lightCamUpVector = Vec4(0, 0, 1, 0);

	auto lookat = [](Vec3 eye, Vec3 lookat, Vec3 up, Vec3* result) -> void {
		result[0] = (lookat - eye).Normalized(); //view dir
		result[1] = up.Normalized();
		result[2] = eye;
		Vec3 right = Cross(result[0], result[1]).Normalized();
		result[1] = Cross(right, result[0]).Normalized();
	};

	Vec3 res[3];

	lookat(lightCamPos.xyz, lightCamViewDir.xyz, lightCamUpVector.xyz, res);

	lightCamPos = Vec4(res[2], 1);
	lightCamViewDir = Vec4(res[0], 0);
	lightCamUpVector = Vec4(res[1], 0);

	uniformsCBData.lightCamPos = lightCamPos;
	uniformsCBData.lightCamViewDir = lightCamViewDir;
	uniformsCBData.lightCamUpVector = lightCamUpVector;

	//TODO get from somewhere
	uniformsCBData.texSize = 2048;

	//create single-frame only cb
	gxeng::VolatileConstBuffer cb = context.CreateVolatileConstBuffer(&uniformsCBData, sizeof(Uniforms));
	cb.SetName("Depth reduction final volatile CB");
	gxeng::ConstBufferView cbv = context.CreateCbv(cb, 0, sizeof(Uniforms));


	commandList.SetResourceState(m_lightMvpUav.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_shadowMxUav.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_csmSplitsUav.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_csmExtentsUav.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS);
	commandList.SetResourceState(m_reductionTexSrv.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

	commandList.SetPipelineState(m_CSO.get());
	commandList.SetComputeBinder(&m_binder);
	commandList.BindCompute(m_reductionBindParam, m_reductionTexSrv);
	commandList.BindCompute(m_outputBindParam0, m_lightMvpUav);
	commandList.BindCompute(m_outputBindParam1, m_shadowMxUav);
	commandList.BindCompute(m_outputBindParam2, m_csmSplitsUav);
	commandList.BindCompute(m_outputBindParam3, m_csmExtentsUav);
	commandList.BindCompute(m_uniformsBindParam, cbv);
	commandList.Dispatch(1, 1, 1);
	commandList.UAVBarrier(m_lightMvpUav.GetResource());
	commandList.UAVBarrier(m_shadowMxUav.GetResource());
	commandList.UAVBarrier(m_csmSplitsUav.GetResource());
	commandList.UAVBarrier(m_csmExtentsUav.GetResource());
}


void DepthReductionFinal::InitRenderTarget(SetupContext& context) {
	if (!m_outputTexturesInited) {
		m_outputTexturesInited = true;

		using gxapi::eFormat;

		auto formatLightMVP = eFormat::R32G32B32A32_FLOAT;
		auto formatShadowMX = eFormat::R32G32B32A32_FLOAT;
		auto formatCSMSplits = eFormat::R32G32_FLOAT;
		auto formatCSMExtents = eFormat::R32G32B32A32_FLOAT;

		gxapi::UavTexture2DArray uavDesc;
		uavDesc.activeArraySize = 1;
		uavDesc.firstArrayElement = 0;
		uavDesc.mipLevel = 0;
		uavDesc.planeIndex = 0;

		gxapi::SrvTexture2DArray srvDesc;
		srvDesc.activeArraySize = 1;
		srvDesc.firstArrayElement = 0;
		srvDesc.numMipLevels = -1;
		srvDesc.mipLevelClamping = 0;
		srvDesc.mostDetailedMip = 0;
		srvDesc.planeIndex = 0;

		//TODO 1D tex
		Texture2D lightMvpTex = context.CreateTexture2D({ 4 * 4, 1, formatLightMVP }, { true, true, false, true });
		lightMvpTex.SetName("Depth reduction final light MVP tex");
		m_lightMvpUav = context.CreateUav(lightMvpTex, formatLightMVP, uavDesc);


		Texture2D shadowMxTex = context.CreateTexture2D({ 4 * 4, 1, formatShadowMX }, { true, true, false, true });
		shadowMxTex.SetName("Depth reduction final shadow MX tex");
		m_shadowMxUav = context.CreateUav(shadowMxTex, formatShadowMX, uavDesc);


		Texture2D csmSplitsTex = context.CreateTexture2D({ 4, 1, formatCSMSplits }, { true, true, false, true });
		csmSplitsTex.SetName("Depth reduction final csm splits tex");
		m_csmSplitsUav = context.CreateUav(csmSplitsTex, formatCSMSplits, uavDesc);


		Texture2D csmExtentsTex = context.CreateTexture2D({ 3 * 4, 1, formatCSMExtents }, { true, true, false, true });
		csmExtentsTex.SetName("Depth reduction final csm extents tex");
		m_csmExtentsUav = context.CreateUav(csmExtentsTex, formatCSMExtents, uavDesc);
	}
}


} // namespace inl::gxeng::nodes
