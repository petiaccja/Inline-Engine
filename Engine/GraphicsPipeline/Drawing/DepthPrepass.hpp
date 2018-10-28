#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: render target, entities, camera
/// </summary>
class DepthPrepass : virtual public GraphicsNode,
					 virtual public GraphicsTask,
					 virtual public InputPortConfig<Texture2D, const BasicCamera*, const EntityCollection<MeshEntity>*>,
					 virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "DepthPrepass"; }
	DepthPrepass();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;

private:
	BindParameter m_transformBindParam;
	gxapi::eFormat m_depthStencilFormat = gxapi::eFormat::UNKNOWN;

	Binder m_binder;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	ShaderProgram m_shader;
	DepthStencilView2D m_targetDsv;
};


} // namespace inl::gxeng::nodes
