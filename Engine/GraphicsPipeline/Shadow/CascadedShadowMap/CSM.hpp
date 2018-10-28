#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>


namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: render target, scene objects, light cascade MVP transform matrices in a texture
/// Output: render target
/// </summary>
class CSM : virtual public GraphicsNode,
			virtual public GraphicsTask,
			virtual public InputPortConfig<Texture2D, const EntityCollection<MeshEntity>*, Texture2D>,
			virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "CSM"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	CSM();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_uniformsBindParam;
	BindParameter m_lightMVPBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	gxapi::eFormat m_depthStencilFormat;

private: // render context
	std::vector<DepthStencilView2D> m_dsvs;
	const EntityCollection<MeshEntity>* m_entities;
	TextureView2D m_lightMVPTexSrv;
};


} // namespace inl::gxeng::nodes
