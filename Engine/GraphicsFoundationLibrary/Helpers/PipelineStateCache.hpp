#pragma once

#include "DynamicPipelineSetup.hpp"

#include "GraphicsEngine_LL/Binder.hpp"
#include "GraphicsEngine_LL/Material.hpp"
#include "GraphicsEngine_LL/ShaderManager.hpp"
#include <BaseLibrary/UniqueIdGenerator.hpp>
#include <GraphicsApi_LL/Common.hpp>
#include <GraphicsApi_LL/IPipelineState.hpp>

#include <InlineMath.hpp>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>


namespace inl::gxeng {
class GraphicsCommandList;
class RenderContext;
class Mesh;
class Material;
class Image;
} // namespace inl::gxeng


namespace inl::gxeng::nodes {


struct PipelineStateTemplate {
	std::string vsFileName;
	std::string gsFileName;
	std::string hsFileName;
	std::string dsFileName;
	std::string psFileName;

	gxapi::StreamOutputState streamOutput;
	gxapi::RasterizerState rasterization;
	gxapi::DepthStencilState depthStencilState;
	gxapi::BlendState blending;
	unsigned blendSampleMask = 0xFFFFFFFF;

	gxapi::ePrimitiveTopologyType primitiveTopologyType = gxapi::ePrimitiveTopologyType::TRIANGLE;
	gxapi::eTriangleStripCutIndex triangleStripCutIndex = gxapi::eTriangleStripCutIndex::DISABLED;

	unsigned numRenderTargets = 1;
	gxapi::eFormat renderTargetFormats[8] = {
		gxapi::eFormat::UNKNOWN,
		gxapi::eFormat::UNKNOWN,
		gxapi::eFormat::UNKNOWN,
		gxapi::eFormat::UNKNOWN,
		gxapi::eFormat::UNKNOWN,
		gxapi::eFormat::UNKNOWN,
		gxapi::eFormat::UNKNOWN,
		gxapi::eFormat::UNKNOWN,
	};
	gxapi::eFormat depthStencilFormat = gxapi::eFormat::UNKNOWN;
	unsigned multisampleCount = 1;
	unsigned multisampleQuality = 0;
};



class PipelineStateConfig {
public:
	PipelineStateConfig(std::unique_ptr<gxapi::IPipelineState> pso,
			  Binder binder,
			  std::vector<BindParameterDesc> materialConstantParams,
			  std::vector<BindParameterDesc> materialTextureParams,
			  std::vector<MaterialCbufferElement> materialCbufferElements)
		: m_pso(std::move(pso)),
		  m_binder(std::move(binder)),
		  m_materialConstantParams(std::move(materialConstantParams)),
		  m_materialTextureParams(std::move(materialTextureParams)),
		  m_materialCbufferElements(std::move(materialCbufferElements)) {}

	void BindPipeline(GraphicsCommandList& list) const;
	void BindMaterial(GraphicsCommandList& list, const Material& material) const;
	const Binder& GetBinder() const;
	gxapi::IPipelineState* GetPSO() const;

private:
	static void UpdateParamColor(const Material::Parameter& param, const MaterialCbufferElement& desc, std::vector<uint8_t>& cbuffer);
	static void UpdateParamValue(const Material::Parameter& param, const MaterialCbufferElement& desc, std::vector<uint8_t>& cbuffer);
	static void UpdateParamOptional(const Material::Parameter& param, const MaterialCbufferElement& desc, std::vector<uint8_t>& cbuffer);
	static void BindParamImage(GraphicsCommandList& list, const Material::Parameter& param, const BindParameter& slot);
	static void BindConstant(GraphicsCommandList& list, const BindParameter& slot, const std::vector<uint8_t>& cbuffer);

private:
	std::unique_ptr<gxapi::IPipelineState> m_pso;
	Binder m_binder;
	std::vector<BindParameterDesc> m_materialConstantParams;
	std::vector<BindParameterDesc> m_materialTextureParams;
	std::vector<MaterialCbufferElement> m_materialCbufferElements;
};



class PipelineStateCache {
private:
	struct StateKey {
		UniqueId materialShaderId;
		UniqueId streamLayoutId;
		bool operator==(const StateKey& rhs) const {
			return materialShaderId == rhs.materialShaderId && streamLayoutId == rhs.streamLayoutId;
		}
	};
	struct StateKeyHash {
		size_t operator()(const StateKey& obj) const {
			return CombineHash(std::hash<UniqueId>()(obj.materialShaderId), std::hash<UniqueId>()(obj.streamLayoutId));
		}
	};

public:
	PipelineStateCache(std::vector<BindParameterDesc> originalBindParams, PipelineStateTemplate psoTemplate);

	void Reset(std::vector<BindParameterDesc> originalBindParams, PipelineStateTemplate psoTemplate);
	const PipelineStateConfig& GetConfig(RenderContext& context, const Mesh& mesh, const Material& material);
	const PipelineStateTemplate& GetTemplate() const;
	
private:
	PipelineStateConfig CreateConfig(RenderContext& context, const Mesh& mesh, const Material& material) const;

private:
	// Should use these caches:
	//std::unordered_map<UniqueId, ShaderProgram> m_vertexShaderCache; // Mesh element list -> Vertex shader.
	//std::unordered_map<UniqueId, ShaderProgram> m_materialShaderCache; // Material shader -> Pixel shader.

	std::unordered_map<StateKey, std::unique_ptr<PipelineStateConfig>, StateKeyHash> m_configCache; // Mesh layout & mtl shader -> PSO.

	std::vector<BindParameterDesc> m_originalBindParams;
	PipelineStateTemplate m_psoTemplate;
};


} // namespace inl::gxeng::nodes