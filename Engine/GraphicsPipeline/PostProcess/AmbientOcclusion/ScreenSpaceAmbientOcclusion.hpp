#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


class ScreenSpaceAmbientOcclusion : virtual public GraphicsNode,
									virtual public GraphicsTask,
									virtual public InputPortConfig<Texture2D, const BasicCamera*, Texture2D>,
									virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "ScreenSpaceAmbientOcclusion"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	ScreenSpaceAmbientOcclusion();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_depthTexBindParam;
	BindParameter m_inputTexBindParam;
	BindParameter m_temporalTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	ShaderProgram m_horizontalShader;
	ShaderProgram m_verticalShader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	std::unique_ptr<gxapi::IPipelineState> m_horizontalPSO;
	std::unique_ptr<gxapi::IPipelineState> m_vertical0PSO;
	std::unique_ptr<gxapi::IPipelineState> m_vertical1PSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_ssaoRtv;
	RenderTargetView2D m_blurHorizontalRtv;
	RenderTargetView2D m_blurVertical0Rtv;
	RenderTargetView2D m_blurVertical1Rtv;
	TextureView2D m_ssaoSrv;
	TextureView2D m_blurHorizontalSrv;
	TextureView2D m_blurVertical0Srv;
	TextureView2D m_blurVertical1Srv;

	Mat44 m_prevVP;

protected: // render context
	TextureView2D m_depthTexSrv;
	const BasicCamera* m_camera;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
