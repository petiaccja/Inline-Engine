#include "Node_ForwardRender.hpp"

#include "NodeUtility.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../NodeContext.hpp"
#include "../GraphicsCommandList.hpp"
#include "../ResourceView.hpp"

#include <array>

namespace inl::gxeng::nodes {

struct light_data
{
	Vec4_Packed diffuse_color;
	Vec4_Packed vs_position;
	Vec4_Packed attenuation_end;
};

struct Uniforms
{
	light_data ld[10];
	Vec4_Packed screen_dimensions;
	Vec4_Packed vs_cam_pos;
	int group_size_x, group_size_y;
	float halfExposureFramerate, //0.5 * exposure time (% of time exposure is open -> 0.75?) * frame rate (s? or fps?)
		  maxMotionBlurRadius; //pixels
};

static void SetWorkgroupSize(unsigned w, unsigned h, unsigned groupSizeW, unsigned groupSizeH, unsigned& dispatchW, unsigned& dispatchH)
{
	//set up work group sizes
	unsigned gw = 0, gh = 0, count = 1;

	while (gw < w)
	{
		gw = groupSizeW * count;
		count++;
	}

	count = 1;

	while (gh < h)
	{
		gh = groupSizeH * count;
		count++;
	}

	dispatchW = unsigned(float(gw) / groupSizeW);
	dispatchH = unsigned(float(gh) / groupSizeH);
}

static bool CheckMeshFormat(const Mesh& mesh) {
	for (size_t i = 0; i < mesh.GetNumStreams(); i++) {
		auto& elements = mesh.GetLayout()[0];
		if (elements.size() != 3) return false;
		if (elements[0].semantic != eVertexElementSemantic::POSITION) return false;
		if (elements[1].semantic != eVertexElementSemantic::NORMAL) return false;
		if (elements[2].semantic != eVertexElementSemantic::TEX_COORD) return false;
	}

	return true;
}


static void ConvertToSubmittable(
	Mesh* mesh,
	std::vector<const gxeng::VertexBuffer*>& vertexBuffers,
	std::vector<unsigned>& sizes,
	std::vector<unsigned>& strides
) {
	vertexBuffers.clear();
	sizes.clear();
	strides.clear();

	for (int streamID = 0; streamID < mesh->GetNumStreams(); streamID++) {
		vertexBuffers.push_back(&mesh->GetVertexBuffer(streamID));
		sizes.push_back((unsigned)vertexBuffers.back()->GetSize());
		strides.push_back((unsigned)mesh->GetVertexBufferStride(streamID));
	}

	assert(vertexBuffers.size() == sizes.size());
	assert(sizes.size() == strides.size());
}



ForwardRender::ForwardRender() {
	this->GetInput<0>().Set({});
}


void ForwardRender::Initialize(EngineContext & context) {
	GraphicsNode::SetTaskSingle(this);
}

void ForwardRender::Reset() {
	m_rtv = RenderTargetView2D();
	m_velocity_rtv = RenderTargetView2D();
	m_dsv = DepthStencilView2D();
	m_entities = nullptr;
	m_camera = nullptr;
	m_directionalLights = nullptr;

	m_shadowMapTexView = TextureView2D();
	m_shadowMXTexView = TextureView2D();
	m_csmSplitsTexView = TextureView2D();
	m_lightMVPTexView = TextureView2D();

	GetInput<0>().Clear();
	GetInput<1>().Clear();
	GetInput<2>().Clear();
	GetInput<3>().Clear();
	GetInput<4>().Clear();
	GetInput<5>().Clear();
	GetInput<6>().Clear();
	GetInput<7>().Clear();
	GetInput<8>().Clear();
}


void ForwardRender::Setup(SetupContext& context) {
	auto& target = this->GetInput<0>().Get();
	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.firstMipLevel = 0;
	rtvDesc.planeIndex = 0;
	m_rtv = context.CreateRtv(target, target.GetFormat(), rtvDesc);
	

	auto& depthStencil = this->GetInput<1>().Get();
	gxapi::DsvTexture2DArray dsvDesc;
	dsvDesc.activeArraySize = 1;
	dsvDesc.firstArrayElement = 0;
	dsvDesc.firstMipLevel = 0;
	m_dsv = context.CreateDsv(depthStencil, FormatAnyToDepthStencil(depthStencil.GetFormat()), dsvDesc);
	

	m_entities = this->GetInput<2>().Get();

	m_camera = this->GetInput<3>().Get();

	m_directionalLights = this->GetInput<4>().Get();
	assert(m_directionalLights->Size() == 1);

	auto shadowMapTex = this->GetInput<5>().Get();
	this->GetInput<5>().Clear();
	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 4;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;
	m_shadowMapTexView = context.CreateSrv(shadowMapTex, FormatDepthToColor(shadowMapTex.GetFormat()), srvDesc);
	

	srvDesc.activeArraySize = 1;

	auto shadowMXTex = this->GetInput<6>().Get();
	this->GetInput<6>().Clear();
	m_shadowMXTexView = context.CreateSrv(shadowMXTex, shadowMXTex.GetFormat(), srvDesc);
	

	auto csmSplitsTex = this->GetInput<7>().Get();
	this->GetInput<7>().Clear();
	m_csmSplitsTexView = context.CreateSrv(csmSplitsTex, csmSplitsTex.GetFormat(), srvDesc);
	

	auto lightMVPTex = this->GetInput<8>().Get();
	this->GetInput<8>().Clear();
	m_lightMVPTexView = context.CreateSrv(lightMVPTex, lightMVPTex.GetFormat(), srvDesc);
	

	auto lightCullData = this->GetInput<9>().Get();
	this->GetInput<9>().Clear();
	m_lightCullDataView = context.CreateSrv(lightCullData, lightCullData.GetFormat(), srvDesc);
	

	if (!m_velocity_rtv)
	{
		using gxapi::eFormat;

		auto formatVelocity = eFormat::R8G8_UNORM;

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
			target.GetWidth(), target.GetHeight(), formatVelocity
		};

		Texture2D velocity_tex = context.CreateTexture2D(desc, { true, true, false, false });
		velocity_tex.SetName("Forward render Velocity tex");
		m_velocity_rtv = context.CreateRtv(velocity_tex, formatVelocity, rtvDesc);
		
	}

	this->GetOutput<0>().Set(target);
	this->GetOutput<1>().Set(m_velocity_rtv.GetResource());


	/*if (!m_binder.has_value()) {
		BindParameterDesc transformBindParamDesc;
		m_transformBindParam = BindParameter(eBindParameterType::CONSTANT, 0);
		transformBindParamDesc.parameter = m_transformBindParam;
		transformBindParamDesc.constantSize = sizeof(float) * 4 * 4 * 2;
		transformBindParamDesc.relativeAccessFrequency = 0;
		transformBindParamDesc.relativeChangeFrequency = 0;
		transformBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

		BindParameterDesc sunBindParamDesc;
		m_sunBindParam = BindParameter(eBindParameterType::CONSTANT, 1);
		sunBindParamDesc.parameter = m_sunBindParam;
		sunBindParamDesc.constantSize = sizeof(float) * 4 * 2;
		sunBindParamDesc.relativeAccessFrequency = 0;
		sunBindParamDesc.relativeChangeFrequency = 0;
		sunBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		BindParameterDesc albedoBindParamDesc;
		m_albedoBindParam = BindParameter(eBindParameterType::TEXTURE, 0);
		albedoBindParamDesc.parameter = m_albedoBindParam;
		albedoBindParamDesc.constantSize = 0;
		albedoBindParamDesc.relativeAccessFrequency = 0;
		albedoBindParamDesc.relativeChangeFrequency = 0;
		albedoBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		BindParameterDesc shadowMapBindParamDesc;
		m_shadowMapBindParam = BindParameter(eBindParameterType::TEXTURE, 1);
		shadowMapBindParamDesc.parameter = m_shadowMapBindParam;
		shadowMapBindParamDesc.constantSize = 0;
		shadowMapBindParamDesc.relativeAccessFrequency = 0;
		shadowMapBindParamDesc.relativeChangeFrequency = 0;
		shadowMapBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		BindParameterDesc shadowMXBindParamDesc;
		m_shadowMXBindParam = BindParameter(eBindParameterType::TEXTURE, 2);
		shadowMXBindParamDesc.parameter = m_shadowMXBindParam;
		shadowMXBindParamDesc.constantSize = 0;
		shadowMXBindParamDesc.relativeAccessFrequency = 0;
		shadowMXBindParamDesc.relativeChangeFrequency = 0;
		shadowMXBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		BindParameterDesc csmSplitsBindParamDesc;
		m_csmSplitsBindParam = BindParameter(eBindParameterType::TEXTURE, 3);
		csmSplitsBindParamDesc.parameter = m_csmSplitsBindParam;
		csmSplitsBindParamDesc.constantSize = 0;
		csmSplitsBindParamDesc.relativeAccessFrequency = 0;
		csmSplitsBindParamDesc.relativeChangeFrequency = 0;
		csmSplitsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		BindParameterDesc sampBindParamDesc;
		sampBindParamDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
		sampBindParamDesc.constantSize = 0;
		sampBindParamDesc.relativeAccessFrequency = 0;
		sampBindParamDesc.relativeChangeFrequency = 0;
		sampBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		gxapi::StaticSamplerDesc samplerDesc;
		samplerDesc.shaderRegister = 0;
		samplerDesc.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
		samplerDesc.addressU = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressV = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.addressW = gxapi::eTextureAddressMode::WRAP;
		samplerDesc.mipLevelBias = 0.f;
		samplerDesc.registerSpace = 0;
		samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		m_binder = context.CreateBinder({ transformBindParamDesc, sunBindParamDesc, albedoBindParamDesc, shadowMapBindParamDesc, shadowMXBindParamDesc, csmSplitsBindParamDesc, sampBindParamDesc },{ samplerDesc });
	}*/
}


void ForwardRender::Execute(RenderContext& context) {
	if (m_entities == nullptr) {
		return;
	}

	gxeng::GraphicsCommandList& commandList = context.AsGraphics();

	// Set render target
	inl::gxeng::RenderTargetView2D* pRTV[] = { &m_rtv, &m_velocity_rtv };
	commandList.SetResourceState(m_velocity_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_dsv.GetResource(), gxapi::eResourceState::DEPTH_WRITE);
	commandList.SetRenderTargets(2, pRTV, &m_dsv);
	commandList.ClearRenderTarget(m_rtv, gxapi::ColorRGBA(0, 0, 0, 1));
	commandList.ClearRenderTarget(m_velocity_rtv, gxapi::ColorRGBA(0.5, 0.5, 0, 1));

	gxapi::Rectangle rect{ 0, (int)m_rtv.GetResource().GetHeight(), 0, (int)m_rtv.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)rect.right;
	viewport.height = (float)rect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &rect);
	commandList.SetViewports(1, &viewport);

	commandList.SetStencilRef(1); // background is 0, anything other than that is 1

	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	Mat44 view = m_camera->GetViewMatrix();
	Mat44 projection = m_camera->GetProjectionMatrix();
	auto viewProjection = view * projection;
	Mat44 prevView = m_camera->GetPrevViewMatrix();
	auto prevViewProjection = prevView * projection;


	std::vector<const gxeng::VertexBuffer*> vertexBuffers;
	std::vector<unsigned> sizes;
	std::vector<unsigned> strides;

	// Iterate over all entities
	for (const MeshEntity* entity : *m_entities) {
		// Get entity parameters
		Mesh* mesh = entity->GetMesh();
		Material* material = entity->GetMaterial();

		assert(mesh != nullptr);
		assert(material != nullptr);

		// Set pipeline state & binder
		const Mesh::Layout& layout = mesh->GetLayout();
		const MaterialShader* materialShader = material->GetShader();
		assert(materialShader != nullptr);

		ScenarioData& scenario = GetScenario(
			context, layout, *materialShader, m_rtv.GetDescription().format, m_dsv.GetDescription().format);

		commandList.SetPipelineState(scenario.pso.get());
		commandList.SetGraphicsBinder(&scenario.binder);

		commandList.SetResourceState(m_shadowMapTexView.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_shadowMXTexView.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_csmSplitsTexView.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
		commandList.SetResourceState(m_lightMVPTexView.GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });

		commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 500), m_shadowMapTexView);
		commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 501), m_shadowMXTexView);
		commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 502), m_csmSplitsTexView);
		commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 503), m_lightMVPTexView);

		commandList.SetResourceState(m_lightCullDataView.GetResource(), {gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE	});

		commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 600), m_lightCullDataView);

		// Set material parameters
		std::vector<uint8_t> materialConstants(scenario.constantsSize);
		for (size_t paramIdx = 0; paramIdx < material->GetParameterCount(); ++paramIdx) {
			const Material::Parameter& param = (*material)[paramIdx];
			switch (param.GetType()) {
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
			{
				BindParameter bindSlot(eBindParameterType::TEXTURE, scenario.offsets[paramIdx]);
				commandList.SetResourceState(((Image*)param)->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
				commandList.BindGraphics(bindSlot, ((Image*)param)->GetSrv());
				break;
			}
			case eMaterialShaderParamType::COLOR:
			{
				*reinterpret_cast<float*>(materialConstants.data() + scenario.offsets[paramIdx] + 0) = ((Vec4)param).x;
				*reinterpret_cast<float*>(materialConstants.data() + scenario.offsets[paramIdx] + 4) = ((Vec4)param).y;
				*reinterpret_cast<float*>(materialConstants.data() + scenario.offsets[paramIdx] + 8) = ((Vec4)param).z;
				*reinterpret_cast<float*>(materialConstants.data() + scenario.offsets[paramIdx] + 12) = ((Vec4)param).w;
				break;
			}
			case eMaterialShaderParamType::VALUE:
			{
				*reinterpret_cast<float*>(materialConstants.data() + scenario.offsets[paramIdx]) = ((float)param);
				break;
			}
			}
		}
		if (scenario.constantsSize > 0) {
			commandList.BindGraphics(BindParameter(eBindParameterType::CONSTANT, 200), materialConstants.data(), (int)materialConstants.size());
		}

		assert(m_directionalLights->Size() == 1);
		const DirectionalLight* sun = *m_directionalLights->begin();

		// Set vertex and light constants
		VsConstants vsConstants;
		LightConstants lightConstants;
		vsConstants.m = entity->GetTransform();
		vsConstants.mvp = entity->GetTransform() * viewProjection;
		vsConstants.mv = entity->GetTransform() * view;
		vsConstants.v = view;
		vsConstants.p = projection;
		vsConstants.prevMVP = entity->GetPrevTransform() * prevViewProjection;
		Vec4 vsLightDir = Vec4(sun->GetDirection(), 0.0f) * view;
		lightConstants.direction = Vec3(vsLightDir.xyz).Normalized();
		lightConstants.color = sun->GetColor();

		commandList.BindGraphics(BindParameter(eBindParameterType::CONSTANT, 0), &vsConstants, sizeof(vsConstants));
		commandList.BindGraphics(BindParameter(eBindParameterType::CONSTANT, 100), &lightConstants, sizeof(lightConstants));

		Uniforms uniformsCBData;
		uniformsCBData.screen_dimensions = Vec4((float)m_rtv.GetResource().GetWidth(), (float)m_rtv.GetResource().GetHeight(), 0.f, 0.f);
		//uniformsCBData.ld[0].vs_position = Vec4(m_camera->GetPosition() + m_camera->GetLookDirection() * 5.f, 1.0f) * m_camera->GetViewMatrix();
		uniformsCBData.ld[0].vs_position = Vec4(Vec3(0, 0, 1), 1.0f) * m_camera->GetViewMatrix();
		uniformsCBData.ld[0].attenuation_end = Vec4(5.0f, 0.f, 0.f, 0.f);
		uniformsCBData.ld[0].diffuse_color = Vec4(1.f, 0.f, 0.f, 1.f);
		uniformsCBData.vs_cam_pos = Vec4(m_camera->GetPosition(), 1.0f) * m_camera->GetViewMatrix();

		uint32_t dispatchW, dispatchH;
		SetWorkgroupSize((unsigned)m_rtv.GetResource().GetWidth(), (unsigned)m_rtv.GetResource().GetHeight(), 16, 16, dispatchW, dispatchH);

		uniformsCBData.group_size_x = dispatchW;
		uniformsCBData.group_size_y = dispatchH;

		uniformsCBData.halfExposureFramerate = 0.5 * 0.75 * 150; //TODO add measured FPS (or target)
		uniformsCBData.maxMotionBlurRadius = 20;

		commandList.BindGraphics(BindParameter(eBindParameterType::CONSTANT, 600), &uniformsCBData, sizeof(uniformsCBData));

		// Set primitives
		vertexBuffers.clear(); sizes.clear(); strides.clear();
		for (size_t i = 0; i < mesh->GetNumStreams(); ++i) {
			vertexBuffers.push_back(&mesh->GetVertexBuffer(i));
			sizes.push_back((unsigned)mesh->GetVertexBuffer(i).GetSize());
			strides.push_back((unsigned)mesh->GetVertexBufferStride(i));

			commandList.SetResourceState(mesh->GetVertexBuffer(i), gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
		}
		commandList.SetResourceState(mesh->GetIndexBuffer(), gxapi::eResourceState::INDEX_BUFFER);
		commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
		commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());

		// Drawcall
		commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());

	}
}



ForwardRender::ScenarioData& ForwardRender::GetScenario(
	RenderContext& context,
	const Mesh::Layout& layout,
	const MaterialShader& shader,
	gxapi::eFormat renderTargetFormat,
	gxapi::eFormat depthStencilFormat)
{
	std::string shaderCode = shader.GetShaderCode();

	ScenarioDesc key{ layout, shaderCode };
	auto scenarioIt = m_scenarios.find(key);

	// Create scenario PSO if needed
	if (scenarioIt == m_scenarios.end()) {
		auto vsIt = m_vertexShaders.find(layout);
		auto psIt = m_materialShaders.find(shaderCode);

		// Compile vertex shader if needed
		if (vsIt == m_vertexShaders.end()) {
			std::string vsCode = GenerateVertexShader(layout);
			ShaderParts vsParts;
			vsParts.vs = true;
			auto res = m_vertexShaders.insert({ layout, context.CompileShader(vsCode, vsParts, "") });
			vsIt = res.first;
		}

		// Compile pixel shader if needed
		if (psIt == m_materialShaders.end()) {
			std::string psCode = GeneratePixelShader(shader);
			ShaderParts psParts;
			psParts.ps = true;
			auto res = m_materialShaders.insert({ shaderCode, context.CompileShader(psCode, psParts, "") });
			psIt = res.first;
		}

		// Create PSO
		std::unique_ptr<gxapi::IPipelineState> pso;
		std::vector<int> offsets;
		size_t constantsSize;
		Binder binder;

		binder = GenerateBinder(context, shader.GetShaderParameters(), offsets, constantsSize);
		pso = CreatePso(context, binder, vsIt->second.vs, psIt->second.ps, renderTargetFormat, depthStencilFormat);

		auto res = m_scenarios.insert({ key, ScenarioData() });
		scenarioIt = res.first;
		scenarioIt->second.pso = std::move(pso);
		scenarioIt->second.renderTargetFormat = renderTargetFormat;
		scenarioIt->second.depthStencilFormat = depthStencilFormat;
		scenarioIt->second.offsets = std::move(offsets);
		scenarioIt->second.binder = std::move(binder);
		scenarioIt->second.constantsSize = constantsSize;
	}
	else if (scenarioIt->second.renderTargetFormat != renderTargetFormat
		|| scenarioIt->second.depthStencilFormat != depthStencilFormat)
	{
		auto& vs = m_vertexShaders.at(layout).vs;
		auto& ps = m_materialShaders.at(shaderCode).ps;

		auto newPso = CreatePso(context, scenarioIt->second.binder, vs, ps, renderTargetFormat, depthStencilFormat);

		scenarioIt->second.pso = std::move(newPso);
		scenarioIt->second.renderTargetFormat = renderTargetFormat;
		scenarioIt->second.depthStencilFormat = depthStencilFormat;
	}

	return scenarioIt->second;
}





std::string ForwardRender::GenerateVertexShader(const Mesh::Layout& layout) {
	// there's only a single vertex format supported for now
	if (layout.GetStreamCount() <= 0) {
		throw InvalidArgumentException("Meshes must have a single interleaved buffer.");
	}

	auto& elements = layout[0];
	if (elements.size() != 3
		|| elements[0].semantic != eVertexElementSemantic::POSITION
		|| elements[1].semantic != eVertexElementSemantic::NORMAL
		|| elements[2].semantic != eVertexElementSemantic::TEX_COORD)
	{
		throw InvalidArgumentException("Mesh must have 3 attributes: position, normal, texcoord.");
	}

	std::string vertexShader =
		"Texture2D<float4> lightMVPTex : register(t503);"
		"struct VsConstants \n"
		"{\n"
		"	float4x4 MVP;\n"
		"	float4x4 prevMVP;\n"
		"	float4x4 MV;\n"
		"	float4x4 M;\n"
		"	float4x4 V;\n"
		"	float4x4 P;\n"
		"};\n"
		"ConstantBuffer<VsConstants> vsConstants : register(b0);\n"

		"struct PS_Input\n"
		"{\n"
		"	float4 position : SV_POSITION;\n"
		"	float3 normal : NO;\n"
		"	float2 texCoord : TEX_COORD0;\n"
		"	float4 vsPosition : TEX_COORD1;\n"
		"	float3 wsNormal : TEX_COORD2;\n"
		"	float4 prevPosition : TEX_COORD3;\n"
		"	float4 currPosition : TEX_COORD4;\n"
		"};\n"

		"PS_Input VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEX_COORD)\n"
		"{\n"
		"	PS_Input result;\n"
		//"	normal.xyz = normalize(normal.xyz);\n"
		"	float3 viewNormal = mul(normal.xyz, (float3x3)vsConstants.MV);\n"

		"float4x4 light_mvp;\n"
		"float cascade = 0;\n"
		"for (int d = 0; d < 4; ++d)\n"
		"{\n"
		"	light_mvp[d] = lightMVPTex.Load(int3(cascade * 4 + d, 0, 0));\n"
		"}\n"

		"	result.position = mul(position, vsConstants.MVP);\n"
		"	result.prevPosition = mul(position, vsConstants.prevMVP);\n"
		"	result.currPosition = result.position;\n"
		//"	result.position = mul(mul(light_mvp, vsConstants.MV), position);\n"
		//"	result.position = mul(mul(vsConstants.P, mul(light_mvp, vsConstants.M)), position);\n"
		"	result.vsPosition = mul(position, vsConstants.MV);\n"
		"	result.normal = viewNormal;\n"
		"	result.texCoord = texCoord.xy;\n"
		"	result.wsNormal = normal.xyz;\n"

		"	return result;\n"
		"}";

	return vertexShader;
}

std::string ForwardRender::GeneratePixelShader(const MaterialShader& shader) {
	// get material shading function's HLSL code
	std::vector<MaterialShaderParameter> params;
	std::string shadingFunction;

	params = shader.GetShaderParameters();
	shadingFunction = shader.GetShaderCode();

	// rename "main" to something else
	std::stringstream renameMain;
	std::regex renameRegex("main");
	std::regex_replace(std::ostreambuf_iterator<char>(renameMain), shadingFunction.begin(), shadingFunction.end(), renameRegex, "mtl_shader");
	shadingFunction = renameMain.str();

	// structures
	std::string structures =
		"struct PsInput {\n"
		"    float4 ndcPos : SV_POSITION;\n"
		"    float3 viewNormal : NO;\n"
		"	 float2 texCoord : TEX_COORD0;\n"
		"	 float4 vsPosition : TEX_COORD1;\n"
		"	 float3 wsNormal : TEX_COORD2;\n"
		"	 float4 prevPosition : TEX_COORD3;\n"
		"	 float4 currPosition : TEX_COORD4;\n"
		"};\n\n"
		"struct MapColor2D {\n"
		"    Texture2DArray<float4> tex;\n"
		"    SamplerState samp;\n"
		"};\n\n"
		"struct MapValue2D {\n"
		"    Texture2DArray<float> tex;\n"
		"    SamplerState samp;\n"
		"};\n";

	// globals
	std::string globals =
		"static float3 g_lightDir;\n"
		"static float3 g_lightColor;\n"
		"static float3 g_normal;\n"
		"static float3 g_wsNormal;\n"
		"static float4 g_ndcPos;\n"
		"static float4 g_vsPos;\n"
		"static float3 g_tex0;\n";

	// add constant buffer, textures and samplers according to shader parameters
	std::stringstream lightConstantBuffer;
	std::stringstream mtlConstantBuffer;
	std::stringstream textures;

	lightConstantBuffer << "struct LightConstants {\n"
		"    float3 direction;\n"
		"    float3 color;\n"
		"};\n";
	lightConstantBuffer << "ConstantBuffer<LightConstants> lightCb: register(b100); \n";

	mtlConstantBuffer << "struct MtlConstants { \n";
	int numMtlConstants = 0;
	for (size_t i = 0; i < params.size(); ++i) {
		switch (params[i].type) {
			case eMaterialShaderParamType::COLOR:
			{
				mtlConstantBuffer << "    float4 param" << i << "; \n";
				++numMtlConstants;
				break;
			}
			case eMaterialShaderParamType::VALUE:
			{
				mtlConstantBuffer << "    float param" << i << "; \n";
				++numMtlConstants;
				break;
			}
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
			{
				textures << "Texture2DArray<float4> tex" << i << " : register(t" << i << "); \n";
				textures << "SamplerState samp" << i << " : register(s" << i << "); \n";
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
			{
				textures << "Texture2DArray<float> tex" << i << " : register(t" << i << "); \n";
				textures << "SamplerState samp" << i << " : register(s" << i << "); \n";
				break;
			}
		}
	}
	mtlConstantBuffer << "};\n";
	mtlConstantBuffer << "ConstantBuffer<MtlConstants> mtlCb: register(b200); \n";
	if (numMtlConstants == 0) {
		mtlConstantBuffer = std::stringstream();
	}

	// main function
	std::stringstream PSMain;
	PSMain << "struct PS_OUTPUT	{ float4 litColor : SV_Target0;	float2 velocity : SV_Target1; };\n";
	PSMain << "PS_OUTPUT PSMain(PsInput psInput) : SV_TARGET {\n";
	PSMain << "	   PS_OUTPUT result;\n";
	PSMain << "    g_lightDir = lightCb.direction;\n";
	PSMain << "    g_lightColor = lightCb.color;\n";
	PSMain << "    g_lightColor *= get_shadow(psInput.vsPosition);\n";
	PSMain << "    g_normal = normalize(psInput.viewNormal);\n";
	PSMain << "    g_wsNormal = normalize(psInput.wsNormal);\n";
	PSMain << "    g_ndcPos = psInput.ndcPos;\n";
	PSMain << "    g_vsPos = psInput.vsPosition;\n";
	PSMain << "    g_tex0 = float3(psInput.texCoord, 0.0f);\n";
	PSMain << "    result.velocity = encodeVelocity(psInput.currPosition, psInput.prevPosition);\n";
	for (size_t i = 0; i < params.size(); ++i) {
		switch (params[i].type) {
			case eMaterialShaderParamType::COLOR:
			{
				PSMain << "    float4 input" << i << "; \n";
				PSMain << "    input" << i << " = mtlCb.param" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::VALUE:
			{
				PSMain << "    float input" << i << "; \n";
				PSMain << "    input" << i << " = mtlCb.param" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
			{
				PSMain << "    MapColor2D input" << i << "; \n";
				PSMain << "    input" << i << ".tex = tex" << i << "; \n";
				PSMain << "    input" << i << ".samp = samp" << i << "; \n\n";
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
			{
				PSMain << "    MapValue2D input" << i << "; \n";
				PSMain << "    input" << i << ".tex = tex" << i << "; \n";
				PSMain << "    input" << i << ".samp = samp" << i << "; \n\n";
				break;
			}
		}
	}
	PSMain << "    result.litColor = mtl_shader(";
	for (intptr_t i = 0; i < (intptr_t)params.size() - 1; ++i) {
		PSMain << "input" << i << ", ";
	}
	if (params.size() > 0) {
		PSMain << "input" << params.size() - 1;
	}
	PSMain << "); return result; \n} \n";

	return
		std::string()
		+ structures
		+ "\n//-------------------------------------\n\n"
		+ globals
		+ "\n//-------------------------------------\n\n"
		+ lightConstantBuffer.str()
		+ mtlConstantBuffer.str()
		+ "\n//-------------------------------------\n\n"
		+ textures.str()
		+ "\n//-------------------------------------\n\n"
		+ "#include \"CSMSample\"\n"
		+ "#include \"PbrBrdf\"\n"
		+ "#include \"TiledLighting\"\n"
		+ "#include \"EncodeVelocity\"\n"
		+ "\n//-------------------------------------\n\n"
		+ shadingFunction
		+ "\n//-------------------------------------\n\n"
		+ PSMain.str();
}

Binder ForwardRender::GenerateBinder(RenderContext& context, const std::vector<MaterialShaderParameter>& mtlParams, std::vector<int>& offsets, size_t& materialCbSize) {
	int textureRegister = 0;
	int cbSize = 0;
	std::vector<BindParameterDesc> descs;
	offsets.clear();

	for (auto& param : mtlParams) {
		switch (param.type) {
		case eMaterialShaderParamType::BITMAP_COLOR_2D:
		case eMaterialShaderParamType::BITMAP_VALUE_2D:
		{
			BindParameterDesc desc;
			desc.parameter = BindParameter(eBindParameterType::TEXTURE, textureRegister);
			desc.constantSize = 0;
			desc.relativeAccessFrequency = 0;
			desc.relativeChangeFrequency = 0;
			desc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;
			descs.push_back(desc);

			offsets.push_back(desc.parameter.reg);

			++textureRegister;

			break;
		}
		case eMaterialShaderParamType::COLOR:
		{
			cbSize = ((cbSize + 15) / 16) * 16; // correct alignement
			offsets.push_back(cbSize);
			cbSize += 16;

			break;
		}
		case eMaterialShaderParamType::VALUE:
		{
			cbSize = ((cbSize + 3) / 4) * 4; // correct alignement
			offsets.push_back(cbSize);
			cbSize += sizeof(float);

			break;
		}
		default:
			assert(false);;
		}
	}

	BindParameterDesc theSamplerDesc;
	theSamplerDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 500);
	theSamplerDesc.constantSize = 0;
	theSamplerDesc.relativeAccessFrequency = 0;
	theSamplerDesc.relativeChangeFrequency = 0;
	theSamplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	gxapi::StaticSamplerDesc theSamplerParam;
	theSamplerParam.shaderRegister = 500;
	theSamplerParam.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
	theSamplerParam.addressU = gxapi::eTextureAddressMode::CLAMP;
	theSamplerParam.addressV = gxapi::eTextureAddressMode::CLAMP;
	theSamplerParam.addressW = gxapi::eTextureAddressMode::CLAMP;
	theSamplerParam.mipLevelBias = 0.f;
	theSamplerParam.registerSpace = 0;
	theSamplerParam.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc shadowMapBindParamDesc;
	shadowMapBindParamDesc.parameter = BindParameter(eBindParameterType::TEXTURE, 500);
	shadowMapBindParamDesc.constantSize = 0;
	shadowMapBindParamDesc.relativeAccessFrequency = 0;
	shadowMapBindParamDesc.relativeChangeFrequency = 0;
	shadowMapBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc shadowMXBindParamDesc;
	shadowMXBindParamDesc.parameter = BindParameter(eBindParameterType::TEXTURE, 501);
	shadowMXBindParamDesc.constantSize = 0;
	shadowMXBindParamDesc.relativeAccessFrequency = 0;
	shadowMXBindParamDesc.relativeChangeFrequency = 0;
	shadowMXBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc csmSplitsBindParamDesc;
	csmSplitsBindParamDesc.parameter = BindParameter(eBindParameterType::TEXTURE, 502);
	csmSplitsBindParamDesc.constantSize = 0;
	csmSplitsBindParamDesc.relativeAccessFrequency = 0;
	csmSplitsBindParamDesc.relativeChangeFrequency = 0;
	csmSplitsBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc lightMVPBindParamDesc;
	lightMVPBindParamDesc.parameter = BindParameter(eBindParameterType::TEXTURE, 503);
	lightMVPBindParamDesc.constantSize = 0;
	lightMVPBindParamDesc.relativeAccessFrequency = 0;
	lightMVPBindParamDesc.relativeChangeFrequency = 0;
	lightMVPBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

	BindParameterDesc lightCullDataBindParamDesc;
	lightCullDataBindParamDesc.parameter = BindParameter(eBindParameterType::TEXTURE, 600);
	lightCullDataBindParamDesc.constantSize = 0;
	lightCullDataBindParamDesc.relativeAccessFrequency = 0;
	lightCullDataBindParamDesc.relativeChangeFrequency = 0;
	lightCullDataBindParamDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

	BindParameterDesc lightUniformsCbDesc;
	lightUniformsCbDesc.parameter = BindParameter(eBindParameterType::CONSTANT, 600);
	lightUniformsCbDesc.constantSize = sizeof(Uniforms);
	lightUniformsCbDesc.relativeAccessFrequency = 0;
	lightUniformsCbDesc.relativeChangeFrequency = 0;
	lightUniformsCbDesc.shaderVisibility = gxapi::eShaderVisiblity::ALL;

	BindParameterDesc vsCbDesc;
	vsCbDesc.parameter = BindParameter(eBindParameterType::CONSTANT, 0);
	vsCbDesc.constantSize = sizeof(VsConstants);
	vsCbDesc.relativeAccessFrequency = 0;
	vsCbDesc.relativeChangeFrequency = 0;
	vsCbDesc.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

	BindParameterDesc lightCbDesc;
	lightCbDesc.parameter = BindParameter(eBindParameterType::CONSTANT, 100);
	lightCbDesc.constantSize = sizeof(LightConstants);
	lightCbDesc.relativeAccessFrequency = 0;
	lightCbDesc.relativeChangeFrequency = 0;
	lightCbDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc mtlCbDesc;
	mtlCbDesc.parameter = BindParameter(eBindParameterType::CONSTANT, 200);
	mtlCbDesc.constantSize = cbSize;
	mtlCbDesc.relativeAccessFrequency = 0;
	mtlCbDesc.relativeChangeFrequency = 0;
	mtlCbDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc samplerDesc;
	samplerDesc.parameter = BindParameter(eBindParameterType::SAMPLER, 0);
	samplerDesc.constantSize = 0;
	samplerDesc.relativeAccessFrequency = 0;
	samplerDesc.relativeChangeFrequency = 0;
	samplerDesc.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	gxapi::StaticSamplerDesc samplerParam;
	samplerParam.shaderRegister = 0;
	samplerParam.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
	samplerParam.addressU = gxapi::eTextureAddressMode::CLAMP;
	samplerParam.addressV = gxapi::eTextureAddressMode::CLAMP;
	samplerParam.addressW = gxapi::eTextureAddressMode::CLAMP;
	samplerParam.mipLevelBias = 0.f;
	samplerParam.registerSpace = 0;
	samplerParam.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	descs.push_back(vsCbDesc);
	descs.push_back(lightCbDesc);
	descs.push_back(lightUniformsCbDesc);

	descs.push_back(theSamplerDesc);
	descs.push_back(shadowMapBindParamDesc);
	descs.push_back(shadowMXBindParamDesc);
	descs.push_back(csmSplitsBindParamDesc);
	descs.push_back(lightMVPBindParamDesc);

	descs.push_back(lightCullDataBindParamDesc);

	if (cbSize > 0) {
		descs.push_back(mtlCbDesc);
	}

	std::vector<gxapi::StaticSamplerDesc> samplerParams;
	for (int i = 0; i < textureRegister; ++i) {
		samplerDesc.parameter.reg = i;
		samplerParam.shaderRegister = i;
		descs.push_back(samplerDesc);
		samplerParams.push_back(samplerParam);
	}

	samplerParams.push_back(theSamplerParam);

	materialCbSize = cbSize;

	return context.CreateBinder(descs, samplerParams);
}


std::unique_ptr<gxapi::IPipelineState> ForwardRender::CreatePso(
	RenderContext& context,
	Binder& binder,
	ShaderStage& vs,
	ShaderStage & ps,
	gxapi::eFormat renderTargetFormat,
	gxapi::eFormat depthStencilFormat)
{
	std::unique_ptr<gxapi::IPipelineState> result;

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
		gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
		gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
	};

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = binder.GetRootSignature();
	psoDesc.vs = vs;
	psoDesc.ps = ps;
	psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_CCW);
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

	//psoDesc.depthStencilState = gxapi::DepthStencilState(false, true);
	psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
	psoDesc.depthStencilState.depthFunc = gxapi::eComparisonFunction::EQUAL;
	psoDesc.depthStencilState.enableStencilTest = true;
	psoDesc.depthStencilState.stencilReadMask = 0;
	psoDesc.depthStencilState.stencilWriteMask = ~uint8_t(0);
	psoDesc.depthStencilState.ccwFace.stencilFunc = gxapi::eComparisonFunction::ALWAYS;
	psoDesc.depthStencilState.ccwFace.stencilOpOnStencilFail = gxapi::eStencilOp::KEEP;
	psoDesc.depthStencilState.ccwFace.stencilOpOnDepthFail = gxapi::eStencilOp::KEEP;
	psoDesc.depthStencilState.ccwFace.stencilOpOnPass = gxapi::eStencilOp::REPLACE;
	psoDesc.depthStencilState.cwFace = psoDesc.depthStencilState.ccwFace;
	psoDesc.depthStencilFormat = depthStencilFormat;

	psoDesc.numRenderTargets = 2;
	psoDesc.renderTargetFormats[0] = renderTargetFormat;
	psoDesc.renderTargetFormats[1] = m_velocity_rtv.GetResource().GetFormat();

	result.reset(context.CreatePSO(psoDesc));

	return result;
}


} // namespace inl::gxeng::nodes
