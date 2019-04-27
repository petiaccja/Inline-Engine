#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: frame color, frame depth stencil, camera, sun
/// Output: frame color
/// </summary>
class DrawSky : virtual public GraphicsNode,
				virtual public GraphicsTask,
				virtual public InputPortConfig<Texture2D, Texture2D, const BasicCamera*, const EntityCollection<DirectionalLight>*>,
				virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "DrawSky"; }
	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;

private: // execution
	RenderTargetView2D m_rtv;
	DepthStencilView2D m_dsv;

protected:
	Binder m_binder;
	BindParameter m_sunCbBindParam;
	BindParameter m_camCbBindParam;

	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	gxapi::eFormat m_colorFormat = gxapi::eFormat::UNKNOWN;
	gxapi::eFormat m_depthStencilFormat = gxapi::eFormat::UNKNOWN;
};


} // namespace inl::gxeng::nodes
