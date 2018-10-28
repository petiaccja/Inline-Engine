#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


class ScreenSpaceReflection : virtual public GraphicsNode,
							  virtual public GraphicsTask,
							  virtual public InputPortConfig<Texture2D, Texture2D, const BasicCamera*, Texture2D>,
							  virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "ScreenSpaceReflection"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	ScreenSpaceReflection();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_inputTexBindParam;
	BindParameter m_depthTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	ShaderProgram m_downsampleShader;
	ShaderProgram m_blurShader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	std::vector<std::unique_ptr<gxapi::IPipelineState>> m_downsamplePSO;
	std::vector<std::unique_ptr<gxapi::IPipelineState>> m_blurHorizontalPSO;
	std::vector<std::unique_ptr<gxapi::IPipelineState>> m_blurVerticalPSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_ssrRtv;

protected: // render context
	TextureView2D m_inputTexSrv;
	std::vector<RenderTargetView2D> m_inputRtv;
	std::vector<TextureView2D> m_inputSrv;
	std::vector<RenderTargetView2D> m_blurRtv;
	std::vector<TextureView2D> m_blurSrv;
	TextureView2D m_depthTexSrv;
	const BasicCamera* m_camera;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
