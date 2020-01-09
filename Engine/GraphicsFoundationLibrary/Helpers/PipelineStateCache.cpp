#include "PipelineStateCache.hpp"

#include "../Helpers/DynamicPipelineSetup.hpp"

#include <BaseLibrary/Range.hpp>
#include <GraphicsEngine/Resources/Vertex.hpp>
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/Material.hpp>
#include <GraphicsEngine_LL/MaterialShader.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>


namespace inl::gxeng::nodes {



PipelineStateCache::PipelineStateCache(PipelineStateTemplate psoTemplate, std::vector<BindParameterDesc> originalBindParams, std::vector<gxapi::StaticSamplerDesc> staticSamplers)
	: m_originalBindParams(std::move(originalBindParams)), m_psoTemplate(std::move(psoTemplate)) {}


void PipelineStateCache::Reset(PipelineStateTemplate psoTemplate, std::vector<BindParameterDesc> originalBindParams, std::vector<gxapi::StaticSamplerDesc> staticSamplers) {
	m_originalBindParams = std::move(originalBindParams);
	m_psoTemplate = std::move(psoTemplate);
	// Clear all data.
	m_configCache.clear();
}


const PipelineStateConfig& PipelineStateCache::GetConfig(RenderContext& context, const Mesh& mesh, const Material& material) {
	// Mesh + material combo key.
	StateKey key;
	key.streamLayoutId = mesh.GetLayout().GetLayoutId();
	key.materialShaderId = material.GetShader()->GetId();

	// Retrieve old or insert new state desc.
	auto it = m_configCache.find(key);
	if (it == m_configCache.end()) {
		PipelineStateConfig rec = CreateConfig(context, mesh, material);
		auto ins = m_configCache.insert({ key, std::make_unique<PipelineStateConfig>(std::move(rec)) });
		it = ins.first;
	}

	PipelineStateConfig& config = *it->second;

	return config;
}

const PipelineStateTemplate& PipelineStateCache::GetTemplate() const {
	return m_psoTemplate;
}


Binder CreateBinder(RenderContext& context,
                      const std::vector<BindParameterDesc>& originalParams,
                      const std::vector<BindParameterDesc>& materialConstantParams,
                      const std::vector<BindParameterDesc>& materialTextureParams) {
	std::vector<BindParameterDesc> params;

	gxapi::StaticSamplerDesc materialSampler;
	materialSampler.shaderRegister = 0;
	materialSampler.filter = gxapi::eTextureFilterMode::MIN_MAG_MIP_POINT;
	materialSampler.addressU = gxapi::eTextureAddressMode::CLAMP;
	materialSampler.addressV = gxapi::eTextureAddressMode::CLAMP;
	materialSampler.addressW = gxapi::eTextureAddressMode::CLAMP;
	materialSampler.mipLevelBias = 0.f;
	materialSampler.registerSpace = 0;
	materialSampler.shaderVisibility = gxapi::eShaderVisiblity::PIXEL;

	params.insert(params.end(), originalParams.begin(), originalParams.end());
	params.insert(params.end(), materialConstantParams.begin(), materialConstantParams.end());
	params.insert(params.end(), materialTextureParams.begin(), materialTextureParams.end());

	return context.CreateBinder(params, { materialSampler });
}


std::unique_ptr<gxapi::IPipelineState> CreatePSO(RenderContext& context,
												   const PipelineSetupDesc& setupDesc,
												   const PipelineStateTemplate& base,
												   const Binder& binder,
												   const ShaderProgram& shader) {
	gxapi::GraphicsPipelineStateDesc psoDesc;

	psoDesc.rootSignature = binder.GetRootSignature();

	psoDesc.vs = shader.vs;
	psoDesc.gs = shader.gs;
	psoDesc.hs = shader.hs;
	psoDesc.ds = shader.ds;
	psoDesc.ps = shader.ps;

	psoDesc.streamOutput = base.streamOutput;
	psoDesc.rasterization = base.rasterization;
	psoDesc.depthStencilState = base.depthStencilState;
	psoDesc.blending = base.blending;
	psoDesc.blendSampleMask = base.blendSampleMask;

	psoDesc.inputLayout.elements = const_cast<gxapi::InputElementDesc*>(setupDesc.inputLayout.data());
	psoDesc.inputLayout.numElements = (unsigned)setupDesc.inputLayout.size();
	psoDesc.primitiveTopologyType = base.primitiveTopologyType;
	psoDesc.triangleStripCutIndex = base.triangleStripCutIndex;

	psoDesc.numRenderTargets = base.numRenderTargets;
	memcpy(psoDesc.renderTargetFormats, base.renderTargetFormats, sizeof psoDesc.renderTargetFormats);
	psoDesc.depthStencilFormat = base.depthStencilFormat;
	psoDesc.multisampleCount = base.multisampleCount;
	psoDesc.multisampleQuality = base.multisampleQuality;

	psoDesc.addDebugInfo = false;

	std::unique_ptr<gxapi::IPipelineState> result(context.CreatePSO(psoDesc));
	return result;
}


std::string PixelShaderCode(std::string baseCode, const std::string& materialCodeFileName) {
	thread_local const std::regex include(R"(//MATERIAL_CODE_INCLUDE)", std::regex_constants::ECMAScript | std::regex_constants::optimize);

	std::string expr = "#include \"" + materialCodeFileName + ".hlsl\"";
	baseCode = std::regex_replace(baseCode, include, expr);

	return baseCode;
}


std::string MaterialCodeFileName(const Material& material) {
	std::stringstream ss;
	ss << std::hex << std::setw(16) << std::setfill('0') << material.GetShader()->GetId().Value();
	return "material_shader_ " + ss.str();
}


std::string ConcatMacros(const std::vector<std::string>& macros) {
	std::string m;
	for (auto& macro : macros) {
		m += macro + " ";
	}
	return m;
}

PipelineStateConfig PipelineStateCache::CreateConfig(RenderContext& context, const Mesh& mesh, const Material& material) const {
	PipelineSetupTemplate base;
	base.bindParams = m_originalBindParams;
	base.materialSamplerName = "globalSamp";
	base.materialMainName = "MtlMain";
	PipelineSetupDesc pdesc = PipelineSetup(base, mesh, material);

	// Store material shader in shader manager.
	std::string mtlShaderFileName = MaterialCodeFileName(material);
	context.StoreShader(mtlShaderFileName, pdesc.materialCode);

	// Generate shader codes.
	std::string vsStr = context.LoadShader(m_psoTemplate.vsFileName);
	std::string hsStr = m_psoTemplate.hsFileName.empty() ? std::string{} : context.LoadShader(m_psoTemplate.hsFileName);
	std::string dsStr = m_psoTemplate.dsFileName.empty() ? std::string{} : context.LoadShader(m_psoTemplate.dsFileName);
	std::string gsStr = m_psoTemplate.gsFileName.empty() ? std::string{} : context.LoadShader(m_psoTemplate.gsFileName);
	std::string psStr = PixelShaderCode(context.LoadShader(m_psoTemplate.psFileName), mtlShaderFileName);

	// Compile shaders.
	ShaderParts parts;
	ShaderProgram shader;
	parts.vs = true;
	shader.vs = context.CompileShader(vsStr, parts, ConcatMacros(pdesc.vsMacros)).vs;
	parts = {};
	if (!hsStr.empty()) {
		parts.hs = true;
		shader.hs = context.CompileShader(hsStr, parts, ConcatMacros(pdesc.hsMacros)).hs;
	}
	parts = {};
	if (!dsStr.empty()) {
		parts.ds = true;
		shader.ds = context.CompileShader(dsStr, parts, ConcatMacros(pdesc.dsMacros)).ds;
	}
	parts = {};
	if (!gsStr.empty()) {
		parts.gs = true;
		shader.gs = context.CompileShader(gsStr, parts, ConcatMacros(pdesc.gsMacros)).gs;
	}
	parts = {};
	parts.ps = true;
	shader.ps = context.CompileShader(psStr, parts, ConcatMacros(pdesc.psMacros)).ps;

	// Create record.
	auto binder = CreateBinder(context, base.bindParams, pdesc.materialConstantParams, pdesc.materialTextureParams);
	auto pso = CreatePSO(context, pdesc, m_psoTemplate, binder, shader);
	return { std::move(pso), std::move(binder), std::move(pdesc.materialConstantParams), std::move(pdesc.materialTextureParams), std::move(pdesc.materialConstantElements) };
}



//------------------------------------------------------------------------------
// PsoRecord
//------------------------------------------------------------------------------

void PipelineStateConfig::BindPipeline(GraphicsCommandList& list) const {
	list.SetGraphicsBinder(&m_binder);
	list.SetPipelineState(m_pso.get());
}


void PipelineStateConfig::BindMaterial(GraphicsCommandList& list, const Material& material) const {
	size_t constantElementIndex = 0;
	size_t textureSlotIndex = 0;
	std::vector<uint8_t> cbuffer(!m_materialConstantParams.empty() ? m_materialConstantParams.front().constantSize : 0);

	for (auto i : Range(material.GetParameterCount())) {
		const auto& parameter = material[i];
		switch (parameter.GetType()) {
			case eMaterialShaderParamType::COLOR:
				UpdateParamColor(parameter, m_materialCbufferElements[constantElementIndex], cbuffer);
				++constantElementIndex;
				break;
			case eMaterialShaderParamType::VALUE:
				UpdateParamValue(parameter, m_materialCbufferElements[constantElementIndex], cbuffer);
				++constantElementIndex;
				break;
			case eMaterialShaderParamType::BITMAP_COLOR_2D:
				[[fallthrough]];
			case eMaterialShaderParamType::BITMAP_VALUE_2D:
				BindParamImage(list, parameter, m_materialTextureParams[textureSlotIndex].parameter);
				++textureSlotIndex;
				break;
		}
		if (parameter.IsOptional()) {
			UpdateParamOptional(parameter, m_materialCbufferElements[constantElementIndex], cbuffer);
			++constantElementIndex;
		}
	}
	if (!m_materialConstantParams.empty()) {
		BindConstant(list, m_materialConstantParams.front().parameter, cbuffer);
	}
}

const Binder& PipelineStateConfig::GetBinder() const {
	return m_binder;
}

gxapi::IPipelineState* PipelineStateConfig::GetPSO() const {
	return m_pso.get();
}

void PipelineStateConfig::UpdateParamColor(const Material::Parameter& param, const MaterialCbufferElement& desc, std::vector<uint8_t>& cbuffer) {
	if (!param.IsSet()) {
		return;
	}
	Vec4_Packed color = (Vec4)param;
	assert(sizeof(color) == desc.size);
	assert(desc.size + desc.offset <= cbuffer.size());
	assert(desc.structureIndex == 0);
	memcpy(cbuffer.data() + desc.offset, color.Data(), sizeof(color));
}

void PipelineStateConfig::UpdateParamValue(const Material::Parameter& param, const MaterialCbufferElement& desc, std::vector<uint8_t>& cbuffer) {
	if (!param.IsSet()) {
		return;
	}
	float value = (float)param;
	assert(sizeof(value) == desc.size);
	assert(desc.size + desc.offset <= cbuffer.size());
	assert(desc.structureIndex == 0);
	memcpy(cbuffer.data() + desc.offset, &value, sizeof(value));
}

void PipelineStateConfig::UpdateParamOptional(const Material::Parameter& param, const MaterialCbufferElement& desc, std::vector<uint8_t>& cbuffer) {
	uint32_t active = param.IsSet() ? 1 : 0;
	assert(sizeof(active) == desc.size);
	assert(sizeof(active) + desc.optionalOffset <= cbuffer.size());
	memcpy(cbuffer.data() + desc.optionalOffset, &active, sizeof(active));
}

void PipelineStateConfig::BindParamImage(GraphicsCommandList& list, const Material::Parameter& param, const BindParameter& slot) {
	if (!param.IsSet()) {
		return;
	}
	Image* image = (Image*)param;
	if (image == nullptr) {
		return;
	}

	list.SetResourceState(image->GetSrv().GetResource(), { gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE });
	list.BindGraphics(slot, image->GetSrv());
}

void PipelineStateConfig::BindConstant(GraphicsCommandList& list, const BindParameter& slot, const std::vector<uint8_t>& cbuffer) {
	list.BindGraphics(slot, cbuffer.data(), (int)cbuffer.size());
}



} // namespace inl::gxeng::nodes