#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>
#include <GraphicsEngine_LL/Material.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>

#include <optional>

namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: target, depth stencil, entities, camera, directional lights, shadow map, shadowMX, csmSplits, lightMVP, point light shadow map
/// </summary>
class ForwardRender : virtual public GraphicsNode,
					  virtual public GraphicsTask,

					  virtual public InputPortConfig<
						  Texture2D,
						  Texture2D,
						  const EntityCollection<MeshEntity>*,
						  const BasicCamera*,
						  const EntityCollection<DirectionalLight>*,
						  Texture2D,
						  Texture2D,
						  Texture2D>,
					  virtual public OutputPortConfig<Texture2D, Texture2D, Texture2D> {
private:
	struct ScenarioDesc {
		Mesh::Layout layout;
		std::string shader;
	};
	struct ScenarioData {
		std::unique_ptr<gxapi::IPipelineState> pso;
		gxapi::eFormat renderTargetFormat = gxapi::eFormat::UNKNOWN;
		gxapi::eFormat depthStencilFormat = gxapi::eFormat::UNKNOWN;
		Binder binder;
		std::vector<int> offsets;
		size_t constantsSize;
	};
	struct VsConstants {
		Mat44_Packed mvp;
		Mat44_Packed prevMVP;
		Mat44_Packed mv;
		Mat44_Packed m;
		Mat44_Packed v;
		Mat44_Packed p;
	};
	struct LightConstants {
		alignas(16) Vec3_Packed direction;
		alignas(16) Vec3_Packed color;
	};

public:
	static const char* Info_GetName() { return "ForwardRender"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	ForwardRender();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

private:
	static std::string GenerateVertexShader(const Mesh::Layout& layout);
	static std::string GeneratePixelShader(const Material& shader);
	Binder GenerateBinder(RenderContext& context, const Material& mtlParams, std::vector<int>& offsets, size_t& materialCbSize);
	std::unique_ptr<gxapi::IPipelineState> CreatePso(
		RenderContext& context,
		Binder& binder,
		ShaderStage& vs,
		ShaderStage& ps,
		gxapi::eFormat renderTargetFormat,
		gxapi::eFormat depthStencilFormat);

	ScenarioData& GetScenario(
		RenderContext& context,
		const Mesh::Layout& layout,
		const Material& material,
		gxapi::eFormat renderTargetFormat,
		gxapi::eFormat depthStencilFormat);

protected:
	//Binder m_binder;
	BindParameter m_transformBindParam;
	BindParameter m_sunBindParam;
	BindParameter m_albedoBindParam;
	BindParameter m_shadowMapBindParam;
	BindParameter m_shadowMXBindParam;
	BindParameter m_csmSplitsBindParam;

private:
	RenderTargetView2D m_targetRTV;
	RenderTargetView2D m_velocityNormalRTV;
	RenderTargetView2D m_albedoRoughnessMetalnessRTV;
	DepthStencilView2D m_targetDSV;
	const EntityCollection<MeshEntity>* m_entities;
	const BasicCamera* m_camera;
	std::optional<const EntityCollection<DirectionalLight>*> m_directionalLights;

	TextureView2D m_lightCullDataView;
	TextureView2D m_layeredShadowTexView;
	std::optional<TextureView2D> m_screenSpaceShadowTexView;

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
