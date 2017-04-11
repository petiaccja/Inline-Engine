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
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.numMipLevels = 1;
	srvDesc.planeIndex = 0;
	m_shadowMapTexView = context.CreateSrv(shadowMapTex, FormatDepthToColor(shadowMapTex.GetFormat()), srvDesc);

	auto shadowMXTex = this->GetInput<6>().Get();
	this->GetInput<6>().Clear();
	m_shadowMXTexView = context.CreateSrv(shadowMXTex, shadowMXTex.GetFormat(), srvDesc);

	auto csmSplitsTex = this->GetInput<7>().Get();
	this->GetInput<7>().Clear();
	m_csmSplitsTexView = context.CreateSrv(csmSplitsTex, csmSplitsTex.GetFormat(), srvDesc);

	this->GetOutput<0>().Set(target);


	if (!m_binder.has_value()) {
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
	}
}


void ForwardRender::Execute(RenderContext& context) {
	if (m_entities == nullptr) {
		return;
	}

	gxeng::GraphicsCommandList& commandList = context.AsGraphics();

	// Set render target
	auto pRTV = &m_rtv;
	commandList.SetResourceState(m_rtv.GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &pRTV, &m_dsv);
	commandList.ClearRenderTarget(m_rtv, gxapi::ColorRGBA(0, 0, 0, 1));

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

	commandList.SetResourceState(m_dsv.GetResource(), 0, gxapi::eResourceState::DEPTH_WRITE);
	commandList.SetStencilRef(1); // background is 0, anything other than that is 1

	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	mathfu::Matrix4x4f view = m_camera->GetViewMatrixRH();
	mathfu::Matrix4x4f projection = m_camera->GetProjectionMatrixRH();
	auto viewProjection = projection * view;


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

		commandList.SetResourceState(m_shadowMapTexView.GetResource(), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
		commandList.SetResourceState(m_shadowMXTexView.GetResource(), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
		commandList.SetResourceState(m_csmSplitsTexView.GetResource(), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);

		commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 500), m_shadowMapTexView);
		commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 501), m_shadowMXTexView);
		commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 502), m_csmSplitsTexView);

		// Set material parameters
		std::vector<uint8_t> materialConstants(scenario.constantsSize);
		for (size_t paramIdx = 0; paramIdx < material->GetParameterCount(); ++paramIdx) {
			const Material::Parameter& param = (*material)[paramIdx];
			switch (param.GetType()) {
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
			{
				BindParameter bindSlot(eBindParameterType::TEXTURE, scenario.offsets[paramIdx]);
				commandList.BindGraphics(bindSlot, *((Image*)param)->GetSrv());
				break;
			}
			case eMaterialShaderParamType::COLOR:
			{
				*reinterpret_cast<float*>(materialConstants.data() + scenario.offsets[paramIdx] + 0) = ((mathfu::Vector4f)param).x();
				*reinterpret_cast<float*>(materialConstants.data() + scenario.offsets[paramIdx] + 4) = ((mathfu::Vector4f)param).y();
				*reinterpret_cast<float*>(materialConstants.data() + scenario.offsets[paramIdx] + 8) = ((mathfu::Vector4f)param).z();
				*reinterpret_cast<float*>(materialConstants.data() + scenario.offsets[paramIdx] + 12) = ((mathfu::Vector4f)param).w();
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
			commandList.BindGraphics(BindParameter(eBindParameterType::CONSTANT, 200), materialConstants.data(), (int)materialConstants.size(), 0);
		}

		assert(m_directionalLights->Size() == 1);
		const DirectionalLight* sun = *m_directionalLights->begin();

		// Set vertex and light constants
		VsConstants vsConstants;
		LightConstants lightConstants;
		entity->GetTransform().Pack(vsConstants.model);
		(viewProjection * entity->GetTransform()).Pack(vsConstants.mvp);
		(view * entity->GetTransform()).Pack(vsConstants.mv);
		lightConstants.direction = sun->GetDirection().Normalized();
		lightConstants.color = sun->GetColor();

		commandList.BindGraphics(BindParameter(eBindParameterType::CONSTANT, 0), &vsConstants, sizeof(vsConstants), 0);
		commandList.BindGraphics(BindParameter(eBindParameterType::CONSTANT, 100), &lightConstants, sizeof(lightConstants), 0);

		// Set primitives
		vertexBuffers.clear(); sizes.clear(); strides.clear();
		for (size_t i = 0; i < mesh->GetNumStreams(); ++i) {
			vertexBuffers.push_back(&mesh->GetVertexBuffer(i));
			sizes.push_back((unsigned)mesh->GetVertexBuffer(i).GetSize());
			strides.push_back((unsigned)mesh->GetVertexBufferStride(i));
		}
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
		throw std::invalid_argument("Meshes must have a single interleaved buffer.");
	}

	auto& elements = layout[0];
	if (elements.size() != 3
		|| elements[0].semantic != eVertexElementSemantic::POSITION
		|| elements[1].semantic != eVertexElementSemantic::NORMAL
		|| elements[2].semantic != eVertexElementSemantic::TEX_COORD)
	{
		throw std::invalid_argument("Mesh must have 3 attributes: position, normal, texcoord.");
	}

	std::string vertexShader =
		"struct VsConstants \n"
		"{\n"
		"	float4x4 MVP;\n"
		"	float4x4 MV;\n"
		"	float4x4 worldInvTr;"
		"};\n"
		"ConstantBuffer<VsConstants> vsConstants : register(b0);\n"

		"struct PS_Input\n"
		"{\n"
		"	float4 position : SV_POSITION;\n"
		"	float3 normal : NO;\n"
		"	float2 texCoord : TEX_COORD0;\n"
		"	float4 vsPosition : TEX_COORD1;\n"
		"};\n"

		"PS_Input VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEX_COORD)\n"
		"{\n"
		"	PS_Input result;\n"

		"	float3 worldNormal = normalize(mul(vsConstants.worldInvTr, float4(normal.xyz, 0.0)).xyz);\n"

		"	result.position = mul(vsConstants.MVP, position);\n"
		"	result.vsPosition = mul(vsConstants.MV, position);\n"
		"	result.normal = worldNormal;\n"
		"	result.texCoord = texCoord.xy;\n"

		"	return result;\n"
		"}";

	return vertexShader;
}

std::string ForwardRender::GeneratePixelShader(const MaterialShader& shader) {
	std::string code = ::inl::gxeng::MaterialGenPixelShader(shader);
	return code;
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
	theSamplerParam.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_LINEAR;
	theSamplerParam.addressU = gxapi::eTextureAddressMode::WRAP;
	theSamplerParam.addressV = gxapi::eTextureAddressMode::WRAP;
	theSamplerParam.addressW = gxapi::eTextureAddressMode::WRAP;
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
	samplerParam.addressU = gxapi::eTextureAddressMode::WRAP;
	samplerParam.addressV = gxapi::eTextureAddressMode::WRAP;
	samplerParam.addressW = gxapi::eTextureAddressMode::WRAP;
	samplerParam.mipLevelBias = 0.f;
	samplerParam.registerSpace = 0;
	samplerParam.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	descs.push_back(vsCbDesc);
	descs.push_back(lightCbDesc);

	descs.push_back(theSamplerDesc);
	descs.push_back(shadowMapBindParamDesc);
	descs.push_back(shadowMXBindParamDesc);
	descs.push_back(csmSplitsBindParamDesc);

	if (cbSize > 0) {
		descs.push_back(mtlCbDesc);
	}

	std::vector<gxapi::StaticSamplerDesc> samplerParams;
	for (int i = 0; i < textureRegister; ++i) {
		samplerDesc.parameter.reg = i;
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

	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = renderTargetFormat;

	result.reset(context.CreatePSO(psoDesc));

	return result;
}


} // namespace inl::gxeng::nodes
