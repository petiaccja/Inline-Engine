#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

#include <optional>

namespace inl::gxeng::nodes {


class ScreenSpaceShadow : virtual public GraphicsNode,
						  virtual public GraphicsTask,
						  virtual public InputPortConfig<Texture2D, const BasicCamera*>,
						  virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "ScreenSpaceShadow"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	ScreenSpaceShadow();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_inputTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_sssRtv;

protected: // render context
	TextureView2D m_inputTexSrv;
	const BasicCamera* m_camera;

private:
	void InitRenderTarget(SetupContext& context);
};



} // namespace inl::gxeng::nodes
