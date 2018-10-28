#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

#include <optional>

namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: render target, scene objects, light cascade MVP transform matrices in a texture
/// Output: render target
/// </summary>
class ShadowMapGen : virtual public GraphicsNode,
					 virtual public GraphicsTask,
					 virtual public InputPortConfig<Texture2D, const EntityCollection<MeshEntity>*>,
					 virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "ShadowMapGen"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	ShadowMapGen();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shadowGenShader;
	std::unique_ptr<gxapi::IPipelineState> m_shadowGenPSO;
	gxapi::eFormat m_depthStencilFormat;

private: // render context
	std::vector<DepthStencilView2D> m_pointLightDsvs;
	const EntityCollection<MeshEntity>* m_entities;
};


} // namespace inl::gxeng::nodes
