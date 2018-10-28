#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

namespace inl::gxeng::nodes {


class BloomBlur : virtual public GraphicsNode,
				  virtual public GraphicsTask,
				  virtual public InputPortConfig<Texture2D, Vec2>,
				  virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "BloomBlur"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	BloomBlur();

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
	RenderTargetView2D m_blurRtv;

protected: // render context
	TextureView2D m_inputTexSrv;
	Vec2 m_dir;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
