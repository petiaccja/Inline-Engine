#include "RenderForwardSimple.hpp"

#include <BaseLibrary/Range.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/Material.hpp>
#include <GraphicsEngine_LL/MaterialShader.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>
#include <GraphicsEngine_LL/AutoRegisterNode.hpp>

#include <regex>


namespace inl::gxeng::nodes {


INL_REGISTER_GRAPHICS_NODE(RenderForwardSimple)


const BindParameter PipelineStateManager::vsBindParam{ eBindParameterType::CONSTANT, 0, 0 };
const BindParameter PipelineStateManager::psBindParam{ eBindParameterType::CONSTANT, 100, 0 };
const BindParameter PipelineStateManager::mtlBindParam{ eBindParameterType::CONSTANT, 200, 0 };


PipelineStateManager::PipelineStateManager() {}


void PipelineStateManager::SetTextureFormats(gxapi::eFormat renderTargetFormat, gxapi::eFormat depthStencilFormat) {
	m_renderTargetFormat = renderTargetFormat;
	m_depthStencilFormat = depthStencilFormat;
}


auto PipelineStateManager::GetPipelineState(RenderContext& context, const Mesh& mesh, const Material& material) -> const StateDesc& {
	// Mesh + material combo key.
	StateKey key;
	key.streamLayoutId = mesh.GetLayout().GetLayoutId();
	key.materialShaderId = material.GetShader()->GetId();

	// Retrieve old or insert new state desc.
	auto it = m_psoCache.find(key);
	if (it == m_psoCache.end()) {
		StateDesc desc = CreateNewStateDesc(context, mesh, material);
		auto ins = m_psoCache.insert({ key, std::make_unique<StateDesc>(std::move(desc)) });
		it = ins.first;
	}

	StateDesc& desc = *it->second;

	// Recompile PSO if buffer formats have changed. Rest is the same.
	if (desc.renderTargetFormat != m_renderTargetFormat || desc.depthStencilFormat != m_depthStencilFormat) {
		desc.pso = GeneratePSO(context, mesh, desc.binder, desc.shader, desc.renderTargetFormat, desc.depthStencilFormat);
	}

	return desc;
}


void PipelineStateManager::VerifyLayout(const Mesh& mesh) {
	bool hasPos;
	for (auto stream : Range(mesh.GetLayout().GetStreamCount())) {
		for (auto element : mesh.GetLayout()[stream]) {
			if (element.semantic == eVertexElementSemantic::POSITION && element.index == 0) {
				hasPos = true;
				break;
			}
		}
	}
	if (!hasPos) {
		throw InvalidArgumentException("Vertex must have at least position attribute.");
	}
}


std::pair<std::function<std::vector<uint8_t>(const Material&)>, unsigned> PipelineStateManager::GenerateMtlCb(const Material& material) {
	unsigned cbSize = 0;
	std::vector<int> offsets;
	std::vector<int> indexes;

	for (auto i : Range(material.GetParameterCount())) {
		const Material::Parameter& param = material[i];
		switch (param.GetType()) {
			case eMaterialShaderParamType::COLOR:
				cbSize = ((cbSize + 15) / 16) * 16; // correct alignement
				offsets.push_back(cbSize);
				cbSize += 16;
				indexes.push_back(i);
				break;
			case eMaterialShaderParamType::VALUE:
				cbSize = ((cbSize + 3) / 4) * 4; // correct alignement
				offsets.push_back(cbSize);
				cbSize += 4;
				indexes.push_back(i);
				break;
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
				break;
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
				break;
			default:
				assert(false);
		}
		if (param.IsOptional()) {
			// Add bool default parameter select indicator.
			// Bools are 32 bit in HLSL.
			cbSize = ((cbSize + 3) / 4) * 4; // correct alignement
			offsets.push_back(cbSize);
			cbSize += 4;
			indexes.push_back(-i - 1);
		}
	}

	std::function<std::vector<uint8_t>(const Material&)> fun =
		[cbSize, offsets = std::move(offsets), indexes = std::move(indexes)](const Material& material) {
			std::vector<uint8_t> cbData(cbSize);

			for (auto i : Range(offsets.size())) {
				int paramIndex = indexes[i];
				int paramOffset = offsets[i];
				bool optparam = paramIndex < 0;
				paramIndex = paramIndex >= 0 ? paramIndex : -(paramIndex + 1);

				const Material::Parameter& param = material[paramIndex];

				if (!optparam) {
					if (param.IsSet()) {
						auto type = param.GetType();
						if (type == eMaterialShaderParamType::COLOR) {
							*reinterpret_cast<Vec4_Packed*>(cbData.data() + paramOffset) = (Vec4_Packed)param;
						}
						else if (type == eMaterialShaderParamType::VALUE) {
							*reinterpret_cast<float*>(cbData.data() + paramOffset) = (float)param;
						}
						else {
							assert(false); // Images must be optional to appear in the cbuffer.
						}
					}
				}
				else {
					uint32_t isSet = param.IsSet();
					*reinterpret_cast<uint32_t*>(cbData.data() + paramOffset) = isSet;
				}
			}

			return cbData;
		};

	return { fun, cbSize };
}


std::pair<std::function<std::vector<const Image*>(const Material&)>, unsigned> PipelineStateManager::GenerateMtlTex(const Material& material) {
	std::vector<int> indexes;

	for (auto i : Range(material.GetParameterCount())) {
		const Material::Parameter& param = material[i];
		auto type = param.GetType();
		if (type == eMaterialShaderParamType::BITMAP_COLOR_2D || type == eMaterialShaderParamType::BITMAP_VALUE_2D) {
			indexes.push_back(i);
		}
	}
	unsigned numTextures = (unsigned)indexes.size();

	std::function<std::vector<const Image*>(const Material&)> fun = [indexes = std::move(indexes)](const Material& material) {
		std::vector<const Image*> images;

		for (auto i : indexes) {
			const Material::Parameter& param = material[i];
			images.push_back(param.IsSet() ? (const Image*)param : nullptr);
		}

		return images;
	};

	return { fun, numTextures };
}


Binder PipelineStateManager::GenerateBinder(RenderContext& context, unsigned vsCbSize, unsigned psCbSize, unsigned mtlCbSize, unsigned numTextures) {
	std::vector<BindParameterDesc> params;

	gxapi::StaticSamplerDesc sampler;
	sampler.shaderRegister = 0;
	sampler.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
	sampler.addressU = gxapi::eTextureAddressMode::CLAMP;
	sampler.addressV = gxapi::eTextureAddressMode::CLAMP;
	sampler.addressW = gxapi::eTextureAddressMode::CLAMP;
	sampler.mipLevelBias = 0.f;
	sampler.registerSpace = 0;
	sampler.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc vsCbParam;
	vsCbParam.parameter = vsBindParam;
	vsCbParam.constantSize = vsCbSize;
	vsCbParam.shaderVisibility = gxapi::eShaderVisiblity::VERTEX;

	BindParameterDesc psCbParam;
	psCbParam.parameter = psBindParam;
	psCbParam.constantSize = psCbSize;
	psCbParam.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	BindParameterDesc mtlCbParam;
	mtlCbParam.parameter = mtlBindParam;
	mtlCbParam.constantSize = mtlCbSize;
	mtlCbParam.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	params.push_back(vsCbParam);
	params.push_back(psCbParam);
	params.push_back(mtlCbParam);

	for (unsigned i = 0; i < numTextures; ++i) {
		BindParameterDesc texParam;
		texParam.parameter.reg = i;
		texParam.parameter.type = eBindParameterType::TEXTURE;
		texParam.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

		params.push_back(texParam);
	}

	return context.CreateBinder(params, { sampler });
}


std::string PipelineStateManager::GetLayoutShaderMacros(const Mesh& mesh) {
	std::string macros;
	for (auto stream : Range(mesh.GetLayout().GetStreamCount())) {
		for (auto element : mesh.GetLayout()[stream]) {
			if (element.semantic == eVertexElementSemantic::NORMAL && element.index == 0) {
				macros += "HAS_NORMAL=1 ";
			}
			if (element.semantic == eVertexElementSemantic::TANGENT && element.index == 0) {
				macros += "HAS_TANGENT=1 ";
			}
			if (element.semantic == eVertexElementSemantic::BITANGENT && element.index == 0) {
				macros += "HAS_BITANGENT=1 ";
			}
			if (element.semantic == eVertexElementSemantic::COLOR && element.index == 0) {
				macros += "HAS_COLOR=1 ";
			}
			if (element.semantic == eVertexElementSemantic::TEX_COORD && element.index == 0) {
				macros += "HAS_TEXCOORD=1 ";
			}
		}
	}
	return macros;
}


std::string PipelineStateManager::GenerateVertexShader(RenderContext& context, const Mesh& mesh) {
	const auto& layout = mesh.GetLayout();

	return context.LoadShader("RenderForwardSimple");
}


MaterialShaderInput& FindShaderInput(MaterialShader& shader, const Material::Parameter& param) {
	for (auto i : Range(shader.GetNumInputs())) {
		if (shader.GetInput(i)->index == param.GetShaderParamIndex()) {
			return *shader.GetInput(i);
		}
	}
	assert(false); // This is not a soft error but a logical bug.
	std::terminate();
}


std::string PipelineStateManager::GenerateMaterialShader(const Material& material) {
	const MaterialShader& shader = *material.GetShader();

	std::stringstream mtlConstantBuffer;
	std::stringstream mtlTextures;
	std::stringstream mtlMain;

	mtlConstantBuffer << "struct MtlConstants { \n";
	int numMtlConstants = 0;
	int textureReg = 0;
	for (size_t i = 0; i < material.GetParameterCount(); ++i) {
		auto& param = material[i];
		switch (param.GetType()) {
			case eMaterialShaderParamType::COLOR: {
				mtlConstantBuffer << "    float4 param" << i << "; \n";
				++numMtlConstants;
				break;
			}
			case eMaterialShaderParamType::VALUE: {
				mtlConstantBuffer << "    float param" << i << "; \n";
				++numMtlConstants;
				break;
			}
			case eMaterialShaderParamType::BITMAP_COLOR_2D: {
				mtlTextures << "Texture2DArray<float4> tex" << i << " : register(t" << textureReg << "); \n";
				++textureReg;
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D: {
				mtlTextures << "Texture2DArray<float> tex" << i << " : register(t" << textureReg << "); \n";
				++textureReg;
				break;
			}
		}

		if (param.IsOptional()) {
			mtlConstantBuffer << "    bool param" << i << "_opt; \n";
		}
	}
	mtlConstantBuffer << "};\n";
	mtlConstantBuffer << "ConstantBuffer<MtlConstants> mtlCb : register(b200);\n\n";

	mtlMain << "void MtlMain() {\n";
	for (size_t i = 0; i < material.GetParameterCount(); ++i) {
		auto& param = material[i];
		auto& input = *shader.GetInput(param.GetShaderParamIndex());

		std::stringstream ssOptOpen;
		std::stringstream ssOptClose;
		if (param.IsOptional()) {
			ssOptOpen << "    if (mtlCb.param" << i << "_opt) {\n    ";
			ssOptClose << "    } else {\n";
			ssOptClose << "        input" << i << " = " << input.GetDefaultValue() << ";\n";
			ssOptClose << "    }\n\n";
		}

		switch (param.GetType()) {
			case eMaterialShaderParamType::COLOR: {
				mtlMain << "    float4 input" << i << "; \n";
				mtlMain << ssOptOpen.str() << "    input" << i << " = mtlCb.param" << i << "; \n"
						<< ssOptClose.str();
				break;
			}
			case eMaterialShaderParamType::VALUE: {
				mtlMain << "    float input" << i << "; \n";
				mtlMain << ssOptOpen.str() << "    input" << i << " = mtlCb.param" << i << "; \n"
						<< ssOptClose.str();
				break;
			}
			case eMaterialShaderParamType::BITMAP_COLOR_2D: {
				mtlMain << "    MapColor2D input" << i << "; \n";
				mtlMain << ssOptOpen.str();
				mtlMain << "    input" << i << ".tex = tex" << i << "; \n";
				mtlMain << "    input" << i << ".samp = globalSamp"
						<< "; \n\n";
				mtlMain << ssOptClose.str();
				break;
			}
			case eMaterialShaderParamType::BITMAP_VALUE_2D: {
				mtlMain << "    MapValue2D input" << i << "; \n";
				mtlMain << ssOptOpen.str();
				mtlMain << "    input" << i << ".tex = tex" << i << "; \n";
				mtlMain << "    input" << i << ".samp = globalSamp"
						<< "; \n\n";
				mtlMain << ssOptClose.str();
				break;
			}
		}
	}

	size_t numOutParams = 0;
	for (auto i : Range(shader.GetNumOutputs())) {
		if (shader.GetOutput(i)->index >= 0) {
			mtlMain << "    " << shader.GetOutput(i)->GetType() << " " << shader.GetOutput(i)->GetName() << ";\n";
			++numOutParams;
		}
	}

	size_t numParams = shader.GetNumInputs() + numOutParams;
	std::vector<std::string> params(numParams);

	for (auto i : Range(shader.GetNumOutputs())) {
		if (shader.GetOutput(i)->index >= 0) {
			params[shader.GetOutput(i)->index] = shader.GetOutput(i)->GetName();
		}
	}

	for (auto i : Range(shader.GetNumInputs())) {
		params[shader.GetInput(i)->index] = shader.GetInput(i)->GetDefaultValue();
	}

	for (auto i : Range(material.GetParameterCount())) {
		params[material[i].GetShaderParamIndex()] = "input" + std::to_string(i);
	}

	mtlMain << "    main(";
	for (intptr_t i = 0; i < numParams; ++i) {
		mtlMain << params[i];
		if (i < (intptr_t)numParams - 1) {
			mtlMain << ", ";
		}
	}
	mtlMain << ");\n";

	mtlMain << "}\n\n";

	std::string mtlShaderCall = shader.GetShaderCode() + "\n\n\n" + mtlConstantBuffer.str() + mtlTextures.str() + mtlMain.str();

	return mtlShaderCall;
}


std::string PipelineStateManager::GeneratePixelShader(RenderContext& context, const Material& material) {
	std::string psStr = context.LoadShader("RenderForwardSimple");

	thread_local std::regex uncomment(R"(//!1)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
	thread_local std::regex id(R"(################)", std::regex_constants::ECMAScript | std::regex_constants::optimize);

	std::smatch sm;
	std::regex_search(psStr, sm, uncomment);
	auto pos = sm[0].first - psStr.begin();
	for (auto i : Range(pos, pos + 4)) {
		psStr[i] = ' ';
	}

	std::regex_search(psStr, sm, id);
	pos = sm[0].first - psStr.begin();
	std::stringstream ss;
	ss << std::hex << std::setw(16) << std::setfill('0') << material.GetShader()->GetId().Value();
	std::string serial = ss.str();
	for (auto i : Range(pos, pos + 16)) {
		psStr[i] = serial[i - pos];
	}

	return psStr;
}


std::unique_ptr<gxapi::IPipelineState> PipelineStateManager::GeneratePSO(RenderContext& context,
																		 const Mesh& mesh,
																		 const Binder& binder,
																		 const ShaderProgram& shader,
																		 gxapi::eFormat renderTargetFormat,
																		 gxapi::eFormat depthStencilFormat) {
	const Mesh::Layout& layout = mesh.GetLayout();

	std::vector<gxapi::InputElementDesc> inputElements;

	for (auto streamIdx : Range(layout.GetStreamCount())) {
		for (const auto& element : layout[streamIdx]) {
			switch (element.semantic) {
				case eVertexElementSemantic::POSITION:
					inputElements.emplace_back("POSITION", 0, gxapi::eFormat::R32G32B32_FLOAT, streamIdx, element.offset);
					break;
				case eVertexElementSemantic::NORMAL:
					inputElements.emplace_back("NORMAL", 0, gxapi::eFormat::R32G32B32_FLOAT, streamIdx, element.offset);
					break;
				case eVertexElementSemantic::COLOR:
					inputElements.emplace_back("COLOR", 0, gxapi::eFormat::R32G32B32A32_FLOAT, streamIdx, element.offset);
					break;
				case eVertexElementSemantic::TEX_COORD:
					inputElements.emplace_back("TEX_COORD", 0, gxapi::eFormat::R32G32_FLOAT, streamIdx, element.offset);
					break;
				case eVertexElementSemantic::TANGENT:
					inputElements.emplace_back("TANGENT", 0, gxapi::eFormat::R32G32B32_FLOAT, streamIdx, element.offset);
					break;
				case eVertexElementSemantic::BITANGENT:
					inputElements.emplace_back("BITANGENT", 0, gxapi::eFormat::R32G32B32_FLOAT, streamIdx, element.offset);
					break;
			}
		}
	}

	gxapi::GraphicsPipelineStateDesc psoDesc;
	psoDesc.inputLayout.elements = inputElements.data();
	psoDesc.inputLayout.numElements = (unsigned)inputElements.size();
	psoDesc.rootSignature = binder.GetRootSignature();
	psoDesc.vs = shader.vs;
	psoDesc.ps = shader.ps;
	psoDesc.rasterization = gxapi::RasterizerState(gxapi::eFillMode::SOLID, gxapi::eCullMode::DRAW_ALL);
	psoDesc.primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;

	psoDesc.depthStencilState = gxapi::DepthStencilState(true, true);
	psoDesc.depthStencilState.depthFunc = gxapi::eComparisonFunction::LESS_EQUAL;
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

	std::unique_ptr<gxapi::IPipelineState> result(context.CreatePSO(psoDesc));
	return result;
}


PipelineStateManager::StateDesc PipelineStateManager::CreateNewStateDesc(RenderContext& context, const Mesh& mesh, const Material& material) const {
	// Verify if mesh is suitable for rendering.
	VerifyLayout(mesh);

	// Generate shader codes.
	std::string vsStr = GenerateVertexShader(context, mesh);
	std::string mtlStr = GenerateMaterialShader(material);
	std::string psStr = GeneratePixelShader(context, material);

	// Store material shader in shader manager.
	std::stringstream mtlShaderId;
	mtlShaderId << std::hex << std::setw(16) << std::setfill('0') << material.GetShader()->GetId().Value();
	context.StoreShader("material_shader_" + mtlShaderId.str(), mtlStr);

	// Compile shaders.
	std::string macros = GetLayoutShaderMacros(mesh);
	ShaderParts parts;
	ShaderProgram shader;
	parts.vs = true;
	shader.vs = context.CompileShader(vsStr, parts, macros + "VERTEX_SHADER=1").vs;
	parts = {};
	parts.ps = true;
	shader.ps = context.CompileShader(psStr, parts, macros + "PIXEL_SHADER=1").ps;

	// Gen material CB.
	auto [setMaterialCbuffer, mtlCbSize] = GenerateMtlCb(material);
	auto [setMaterielTex, numTextures] = GenerateMtlTex(material);

	StateDesc desc;
	desc.binder = GenerateBinder(context, sizeof(VsConstants), sizeof(PsConstants), mtlCbSize, numTextures);
	desc.shader = std::move(shader);
	desc.materialCbuffer = std::move(setMaterialCbuffer);
	desc.materialTex = std::move(setMaterielTex);
	desc.renderTargetFormat = m_renderTargetFormat;
	desc.depthStencilFormat = m_depthStencilFormat;
	desc.pso = GeneratePSO(context, mesh, desc.binder, desc.shader, desc.renderTargetFormat, desc.depthStencilFormat);
	return desc;
}


void RenderForwardSimple::Reset() {
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}


void RenderForwardSimple::Setup(SetupContext& context) {
	auto renderTarget = GetInput<0>().Get();
	auto depthTarget = GetInput<1>().Get();
	auto camera = GetInput<2>().Get();
	auto entities = GetInput<3>().Get();
	auto directionalLight = GetInput<4>().Get();

	CreateRenderTargetViews(context, renderTarget, depthTarget);
	m_psoManager.SetTextureFormats(renderTarget.GetFormat(), depthTarget.GetFormat());

	GetOutput<0>().Set(renderTarget);
	GetOutput<1>().Set(depthTarget);
}


void RenderForwardSimple::Execute(RenderContext& context) {
	auto renderTarget = GetInput<0>().Get();
	auto depthTarget = GetInput<1>().Get();
	auto camera = GetInput<2>().Get();
	auto entities = GetInput<3>().Get();
	auto directionalLight = GetInput<4>().Get();

	VsConstants vsConstants;
	PsConstants psConstants;

	Mat44 world, view, proj, dworld;
	view = camera->GetViewMatrix();
	proj = camera->GetProjectionMatrix();

	psConstants.lightColor = directionalLight->Size() > 0 ? (*directionalLight->begin())->GetColor() : Vec3{ 0, 0, 0 };
	psConstants.lightDir = directionalLight->Size() > 0 ? (*directionalLight->begin())->GetDirection() : Vec3{ 0, 0, -1 };

	// Request command list.
	GraphicsCommandList& commandList = context.AsGraphics();

	// Set render targets.
	const RenderTargetView2D* renderTargets[] = { &m_rtv };
	commandList.SetResourceState(m_rtv.GetResource(), gxapi::eResourceState::RENDER_TARGET);
	commandList.SetResourceState(m_dsv.GetResource(), gxapi::eResourceState::DEPTH_WRITE);
	commandList.SetRenderTargets(1, renderTargets, &m_dsv);

	commandList.ClearRenderTarget(m_rtv, gxapi::ColorRGBA{ 0, 0, 0, 0 });
	commandList.ClearDepthStencil(m_dsv, 1.0f, 0);

	// Set scissor rects and shit like that.
	gxapi::Rectangle scissorRect{ 0, (int)m_rtv.GetResource().GetHeight(), 0, (int)m_rtv.GetResource().GetWidth() };
	gxapi::Viewport viewport;
	viewport.width = (float)scissorRect.right;
	viewport.height = (float)scissorRect.bottom;
	viewport.topLeftX = 0;
	viewport.topLeftY = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	commandList.SetScissorRects(1, &scissorRect);
	commandList.SetViewports(1, &viewport);

	commandList.SetPrimitiveTopology(gxapi::ePrimitiveTopology::TRIANGLELIST);
	commandList.SetStencilRef(1);

	// Render entities.
	std::vector<const VertexBuffer*> vertexBuffers;
	std::vector<unsigned> vertexBufferSizes;
	std::vector<unsigned> vertexBufferStrides;

	for (auto& entity : *entities) {
		if (!entity->GetMesh() || !entity->GetMaterial()) {
			continue;
		}
		const Mesh& mesh = *entity->GetMesh();
		const Material& material = *entity->GetMaterial();

		const PipelineStateManager::StateDesc& stateDesc = m_psoManager.GetPipelineState(context, mesh, material);
		std::vector<uint8_t> mtlConstants = stateDesc.materialCbuffer(material);
		std::vector<const Image*> mtlTextures = stateDesc.materialTex(material);

		world = entity->GetTransform();
		dworld = entity->GetTransformMotion();
		vsConstants.world = world;
		vsConstants.worldViewProj = world * view * proj;
		vsConstants.worldViewProjDer = dworld * view * proj; // Okay, it's actually not this simple to calculate, I just write something.

		commandList.SetGraphicsBinder(&stateDesc.binder);
		commandList.SetPipelineState(stateDesc.pso.get());

		commandList.BindGraphics(PipelineStateManager::vsBindParam, &vsConstants, sizeof(vsConstants));
		commandList.BindGraphics(PipelineStateManager::psBindParam, &psConstants, sizeof(psConstants));
		commandList.BindGraphics(PipelineStateManager::mtlBindParam, mtlConstants.data(), mtlConstants.size());

		for (auto i : Range(mtlTextures.size())) {
			BindParameter texParam{ eBindParameterType::TEXTURE, (unsigned)i, 0 };
			const Image* image = mtlTextures[i];
			commandList.SetResourceState(image->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
			commandList.BindGraphics(texParam, image->GetSrv());
		}

		vertexBuffers.clear();
		vertexBufferSizes.clear();
		vertexBufferStrides.clear();
		for (auto streamIdx : Range(mesh.GetNumStreams())) {
			vertexBuffers.push_back(&mesh.GetVertexBuffer(streamIdx));
			vertexBufferSizes.push_back(mesh.GetVertexBuffer(streamIdx).GetSize());
			vertexBufferStrides.push_back(mesh.GetVertexBufferStride(streamIdx));
			commandList.SetResourceState(*vertexBuffers[streamIdx], gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER);
		}
		commandList.SetResourceState(mesh.GetIndexBuffer(), gxapi::eResourceState::INDEX_BUFFER);

		commandList.SetVertexBuffers(0, mesh.GetNumStreams(), vertexBuffers.data(), vertexBufferSizes.data(), vertexBufferStrides.data());
		commandList.SetIndexBuffer(&mesh.GetIndexBuffer(), mesh.IsIndexBuffer32Bit());

		commandList.DrawIndexedInstanced(mesh.GetIndexBuffer().GetIndexCount(), 0, 0, 1, 0);
	}
}



const std::string& RenderForwardSimple::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
		"Camera",
		"Entities",
		"Lights",
	};
	return names[index];
}


const std::string& RenderForwardSimple::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
	};
	return names[index];
}


void RenderForwardSimple::CreateRenderTargetViews(SetupContext& context, const Texture2D& rt, const Texture2D& ds) {
	if (!m_rtv || rt != m_rtv.GetResource()) {
		gxapi::RtvTexture2DArray desc;
		desc.activeArraySize = 1;
		desc.firstArrayElement = 0;
		desc.firstMipLevel = 0;
		desc.planeIndex = 0;

		m_rtv = context.CreateRtv(rt, rt.GetFormat(), desc);
	}
	if (!m_dsv || ds != m_dsv.GetResource()) {
		gxapi::DsvTexture2DArray desc;
		desc.activeArraySize = 1;
		desc.firstArrayElement = 0;
		desc.firstMipLevel = 0;

		m_dsv = context.CreateDsv(ds, ds.GetFormat(), desc);
	}
}


} // namespace inl::gxeng::nodes
