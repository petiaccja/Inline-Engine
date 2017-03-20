#include "Node_ForwardRender.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"
#include "../Image.hpp"
#include "../DirectionalLight.hpp"
#include "../GraphicsContext.hpp"

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



ForwardRender::ForwardRender(gxapi::IGraphicsApi * graphicsApi) {
	this->GetInput<0>().Set({});

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

	m_binder = Binder{ graphicsApi,{ transformBindParamDesc, sunBindParamDesc, albedoBindParamDesc, shadowMapBindParamDesc, shadowMXBindParamDesc, csmSplitsBindParamDesc, sampBindParamDesc },{ samplerDesc } };
}


void ForwardRender::InitGraphics(const GraphicsContext& context) {
	m_graphicsContext = context;

	auto swapChainDesc = context.GetSwapChainDesc();
	InitRenderTarget(swapChainDesc.width, swapChainDesc.height);

	ShaderParts shaderParts;
	shaderParts.vs = true;
	shaderParts.ps = true;

	auto shader = m_graphicsContext.CreateShader("ForwardRender", shaderParts, "");

	std::vector<gxapi::InputElementDesc> inputElementDesc = {
		gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
		gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
		gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
	};

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElementDesc.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
	psoDesc.rootSignature = m_binder.GetRootSignature();
	psoDesc.vs = shader.vs;
	psoDesc.ps = shader.ps;
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
	psoDesc.depthStencilFormat = gxapi::eFormat::D32_FLOAT_S8X24_UINT;

	psoDesc.numRenderTargets = 1;
	psoDesc.renderTargetFormats[0] = gxapi::eFormat::R16G16B16A16_FLOAT;

	m_PSO.reset(m_graphicsContext.CreatePSO(psoDesc));
}


Task ForwardRender::GetTask() {
	return Task({ [this](const ExecutionContext& context) {
		ExecutionResult result;

		auto depthStencil = this->GetInput<0>().Get();
		this->GetInput<0>().Clear();

		const EntityCollection<MeshEntity>* entities = this->GetInput<1>().Get();
		this->GetInput<1>().Clear();

		const BasicCamera* camera = this->GetInput<2>().Get();
		this->GetInput<2>().Clear();

		const DirectionalLight* sun = this->GetInput<3>().Get();
		this->GetInput<3>().Clear();

		gxeng::pipeline::Texture2D shadowMapTex = this->GetInput<4>().Get();
		this->GetInput<4>().Clear();

		gxeng::pipeline::Texture2D shadowMXTex = this->GetInput<5>().Get();
		this->GetInput<5>().Clear();

		gxeng::pipeline::Texture2D csmSplitsTex = this->GetInput<6>().Get();
		this->GetInput<6>().Clear();

		this->GetOutput<0>().Set(pipeline::Texture2D(m_renderTargetSrv, m_rtv));

		if (entities) {
			GraphicsCommandList cmdList = context.GetGraphicsCommandList();

			DepthStencilView2D dsv = depthStencil.QueryDepthStencil(cmdList, m_graphicsContext);

			RenderScene(dsv, *entities, camera, sun, shadowMapTex, shadowMXTex, csmSplitsTex, cmdList);
			result.AddCommandList(std::move(cmdList));
		}

		return result;
	} });
}


void ForwardRender::InitRenderTarget(unsigned width, unsigned height) {
	auto format = gxapi::eFormat::R16G16B16A16_FLOAT;

	Texture2D tex = m_graphicsContext.CreateRenderTarget2D(width, height, format, true);

	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.planeIndex = 0;
	rtvDesc.firstMipLevel = 0;
	m_rtv = m_graphicsContext.CreateRtv(tex, format, rtvDesc);

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.numMipLevels = -1;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.planeIndex = 0;
	m_renderTargetSrv = m_graphicsContext.CreateSrv(tex, format, srvDesc);
}


void ForwardRender::RenderScene(
	DepthStencilView2D& dsv,
	const EntityCollection<MeshEntity>& entities,
	const BasicCamera* camera,
	const DirectionalLight* sun,
	pipeline::Texture2D& shadowMapTex,
	pipeline::Texture2D& shadowMXTex,
	pipeline::Texture2D& csmSplitsTex,
	GraphicsCommandList& commandList
) {
	// Set render target
	auto pRTV = &m_rtv;
	commandList.SetResourceState(m_rtv.GetResource(), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &pRTV, &dsv);
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

	commandList.SetResourceState(dsv.GetResource(), 0, gxapi::eResourceState::DEPTH_WRITE);
	commandList.SetStencilRef(1); // background is 0, anything other than that is 1

	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);

	mathfu::Matrix4x4f view = camera->GetViewMatrixRH();
	mathfu::Matrix4x4f projection = camera->GetProjectionMatrixRH();
	auto viewProjection = projection * view;


	std::vector<const gxeng::VertexBuffer*> vertexBuffers;
	std::vector<unsigned> sizes;
	std::vector<unsigned> strides;

	// Iterate over all entities
	for (const MeshEntity* entity : entities) {
		// Get entity parameters
		Mesh* mesh = entity->GetMesh();
		Material* material = entity->GetMaterial();

		if (material != nullptr) {
			// Set pipeline state & binder
			const Mesh::Layout& layout = mesh->GetLayout();
			const MaterialShader* materialShader = material->GetShader();
			assert(materialShader != nullptr);

			ScenarioData& scenario = GetScenario(layout, *materialShader);

			commandList.SetPipelineState(scenario.pso.get());
			commandList.SetGraphicsBinder(&scenario.binder);

			commandList.SetResourceState(const_cast<Texture2D&>(shadowMapTex.QueryRead().GetResource()), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
			commandList.SetResourceState(const_cast<Texture2D&>(shadowMXTex.QueryRead().GetResource()), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);
			commandList.SetResourceState(const_cast<Texture2D&>(csmSplitsTex.QueryRead().GetResource()), 0, gxapi::eResourceState::PIXEL_SHADER_RESOURCE);

			commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 500), shadowMapTex.QueryRead());
			commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 501), shadowMXTex.QueryRead());
			commandList.BindGraphics(BindParameter(eBindParameterType::TEXTURE, 502), csmSplitsTex.QueryRead());

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
		else {
			// THIS PATH IS USED TO BYPASS MATERIAL SYSTEM AND RENDER ENTITIES WITH SIMPLY A TEXTURE
			// THIS IS DEPRECATED, REMOVE IT ASAP!
			commandList.SetPipelineState(m_PSO.get());
			commandList.SetGraphicsBinder(&m_binder);

			{
				std::array<mathfu::VectorPacked<float, 4>, 2> sunCBData;
				auto sunDir = mathfu::Vector4f(sun->GetDirection(), 0.0);
				auto sunColor = mathfu::Vector4f(sun->GetColor(), 0.0);

				sunDir.Pack(sunCBData.data());
				sunColor.Pack(sunCBData.data() + 1);
				commandList.BindGraphics(m_sunBindParam, sunCBData.data(), sizeof(sunCBData), 0);
			}

			// Draw mesh
			if (!CheckMeshFormat(*mesh)) {
				continue;
			}

			ConvertToSubmittable(mesh, vertexBuffers, sizes, strides);

			auto world = entity->GetTransform();
			auto MVP = viewProjection * world;
			auto worldInvTr = world.Inverse().Transpose();

			std::array<mathfu::VectorPacked<float, 4>, 8> transformCBData;
			MVP.Pack(transformCBData.data());
			worldInvTr.Pack(transformCBData.data() + 4);

			commandList.BindGraphics(m_albedoBindParam, *entity->GetTexture()->GetSrv());
			commandList.BindGraphics(m_transformBindParam, transformCBData.data(), sizeof(transformCBData), 0);

			commandList.SetVertexBuffers(0, (unsigned)vertexBuffers.size(), vertexBuffers.data(), sizes.data(), strides.data());
			commandList.SetIndexBuffer(&mesh->GetIndexBuffer(), mesh->IsIndexBuffer32Bit());
			commandList.DrawIndexedInstanced((unsigned)mesh->GetIndexBuffer().GetIndexCount());
		}
	}
}



ForwardRender::ScenarioData& ForwardRender::GetScenario(const Mesh::Layout& layout, const MaterialShader& shader) {
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
			auto res = m_vertexShaders.insert({ layout, m_graphicsContext.CompileShader(vsCode, vsParts, "") });
			vsIt = res.first;
		}

		// Compile pixel shader if needed
		if (psIt == m_materialShaders.end()) {
			std::string psCode = GeneratePixelShader(shader);
			ShaderParts psParts;
			psParts.ps = true;
			auto res = m_materialShaders.insert({ shaderCode, m_graphicsContext.CompileShader(psCode, psParts, "") });
			psIt = res.first;
		}

		// Create PSO
		std::vector<gxapi::InputElementDesc> inputElementDesc = {
			gxapi::InputElementDesc("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 0),
			gxapi::InputElementDesc("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, 0, 12),
			gxapi::InputElementDesc("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, 0, 24),
		};


		std::unique_ptr<gxapi::IPipelineState> pso;
		std::vector<int> offsets;
		size_t constantsSize;
		Binder binder;

		binder = GenerateBinder(shader.GetShaderParameters(), offsets, constantsSize);

		gxapi::GraphicsPipelineStateDesc psoDesc;
		psoDesc.inputLayout.elements = inputElementDesc.data();
		psoDesc.inputLayout.numElements = (unsigned)inputElementDesc.size();
		psoDesc.rootSignature = binder.GetRootSignature();
		psoDesc.vs = vsIt->second.vs;
		psoDesc.ps = psIt->second.ps;
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
		psoDesc.depthStencilFormat = gxapi::eFormat::D32_FLOAT_S8X24_UINT;

		psoDesc.numRenderTargets = 1;
		psoDesc.renderTargetFormats[0] = gxapi::eFormat::R16G16B16A16_FLOAT;

		pso.reset(m_graphicsContext.CreatePSO(psoDesc));

		auto res = m_scenarios.insert({ key, ScenarioData() });
		scenarioIt = res.first;
		scenarioIt->second.pso = std::move(pso);
		scenarioIt->second.offsets = std::move(offsets);
		scenarioIt->second.binder = std::move(binder);
		scenarioIt->second.constantsSize = constantsSize;
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

Binder ForwardRender::GenerateBinder(const std::vector<MaterialShaderParameter>& mtlParams, std::vector<int>& offsets, size_t& materialCbSize) {
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

	return m_graphicsContext.CreateBinder(descs, samplerParams);
}


} // namespace inl::gxeng::nodes
