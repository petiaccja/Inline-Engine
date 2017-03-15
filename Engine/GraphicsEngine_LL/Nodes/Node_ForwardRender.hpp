#pragma once

#include "../GraphicsNode.hpp"

#include "Node_GenCSM.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../Mesh.hpp"
#include "../Material.hpp"
#include "../ConstBufferHeap.hpp"
#include "../GraphicsContext.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

namespace inl::gxeng::nodes {

class ForwardRender :
	virtual public GraphicsNode,
	// Inputs: depth stencil (from depth prepass), geometry, camera, sun
	virtual public exc::InputPortConfig<pipeline::Texture2D, const EntityCollection<MeshEntity>*, const Camera*, const DirectionalLight*, pipeline::Texture2D, pipeline::Texture2D, pipeline::Texture2D>,
	virtual public exc::OutputPortConfig<pipeline::Texture2D>
{
private:
	struct ScenarioDesc {
		Mesh::Layout layout;
		std::string shader;
	};
	struct ScenarioData {
		std::unique_ptr<gxapi::IPipelineState> pso;
		Binder binder;
		std::vector<int> offsets;
		size_t constantsSize;
	};
	struct VsConstants {
		mathfu::VectorPacked<float, 4> mvp[4];
		mathfu::VectorPacked<float, 4> mv[4];
		mathfu::VectorPacked<float, 4> model[4];
	};	
	struct LightConstants {
		alignas(16) mathfu::VectorPacked<float, 3> direction;
		alignas(16) mathfu::VectorPacked<float, 3> color;
	};
public:
	ForwardRender(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;

private:
	void InitRenderTarget(unsigned width, unsigned height);
	void RenderScene(
		DepthStencilView2D& dsv,
		const EntityCollection<MeshEntity>& entities,
		const Camera* camera,
		const DirectionalLight* sun,
		pipeline::Texture2D& shadowMapTex,
		pipeline::Texture2D& shadowMXTex,
		pipeline::Texture2D& csmSplitsTex,
		GraphicsCommandList& commandList);

	static std::string GenerateVertexShader(const Mesh::Layout& layout);
	static std::string GeneratePixelShader(const MaterialShader& shader);
	Binder GenerateBinder(const std::vector<MaterialShaderParameter>& mtlParams, std::vector<int>& offsets, size_t& materialCbSize);
	ScenarioData& GetScenario(const Mesh::Layout& layout, const MaterialShader& shader);
protected:
	//unsigned m_width;
	//unsigned m_height;

	RenderTargetView2D m_rtv;
	TextureView2D m_renderTargetSrv;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_transformBindParam;
	BindParameter m_sunBindParam;
	BindParameter m_albedoBindParam;
	BindParameter m_shadowMapBindParam;
	BindParameter m_shadowMXBindParam;
	BindParameter m_csmSplitsBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
private:
	struct ElementHash {
		size_t operator()(const Mesh::Layout& obj) const { return obj.GetElementHash(); }
		size_t operator()(const Mesh::Layout& lhs, const Mesh::Layout& rhs) const { return lhs.EqualElements(rhs); }
	};
	struct ScenarioHash {
		size_t operator()(const ScenarioDesc& obj) const { return obj.layout.GetLayoutHash() ^ std::hash<std::string>()(obj.shader); }
		size_t operator()(const ScenarioDesc& lhs, const ScenarioDesc& rhs) const { 
			return lhs.layout.EqualLayout(rhs.layout) && lhs.shader == rhs.shader;
		}
	};
	std::unordered_map<std::string, ShaderProgram> m_materialShaders; // maps MaterialShader codes to pixel shaders
	std::unordered_map<Mesh::Layout, ShaderProgram, ElementHash, ElementHash> m_vertexShaders; // maps Mesh layouts to vertex shaders
	std::unordered_map<ScenarioDesc, ScenarioData, ScenarioHash, ScenarioHash> m_scenarios; // maps mesh-mtlshader pairs to PSOs
};

} // namespace inl::gxeng::nodes

