#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


class ShadowFilter : virtual public GraphicsNode,
					 virtual public GraphicsTask,
					 virtual public InputPortConfig<Texture2D, Texture2D, Texture2D, Texture2D, Texture2D, Texture2D, const BasicCamera*>,
					 virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "ShadowFilter"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	ShadowFilter();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_inputTexBindParam;
	BindParameter m_csmTexBindParam;
	BindParameter m_shadowMxTexBindParam;
	BindParameter m_csmSplitsTexBindParam;
	BindParameter m_lightMvpTexBindParam;
	BindParameter m_cubeShadowTexBindParam;
	BindParameter m_csmMinfilterTexBindParam;
	BindParameter m_cubeMinfilterTexBindParam;
	BindParameter m_shadowLayersTexBindParam;
	BindParameter m_penumbraLayersTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_minfilterShader;
	ShaderProgram m_penumbraShader;
	ShaderProgram m_blurShader;
	std::unique_ptr<gxapi::IPipelineState> m_minfilterPSO;
	std::unique_ptr<gxapi::IPipelineState> m_penumbraPSO;
	std::unique_ptr<gxapi::IPipelineState> m_blurPSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	std::vector<TextureView2D> m_inputTexSrv;
	std::vector<RenderTargetView2D> m_filterHorizontalRtv;
	std::vector<TextureView2D> m_filterHorizontalSrv;
	std::vector<RenderTargetView2D> m_filterVerticalRtv;
	std::vector<TextureViewCube> m_cubeMinfilterSrv;
	std::vector<TextureView2D> m_csmMinfilterSrv;

	RenderTargetView2D m_shadowLayersRtv;
	RenderTargetView2D m_penumbraLayersRtv;
	RenderTargetView2D m_blurLayersFirstPassRtv;
	RenderTargetView2D m_blurLayersSecondPassRtv;

protected: // render context
	TextureView2D m_csmTexSrv;
	TextureView2D m_shadowMxTexSrv;
	TextureView2D m_csmSplitsTexSrv;
	TextureView2D m_lightMvpTexSrv;
	TextureView2D m_depthTexSrv;
	TextureViewCube m_cubeShadowTexSrv;
	TextureView2D m_shadowLayersSrv;
	TextureView2D m_penumbraLayersSrv;
	TextureView2D m_blurLayersFirstPassSrv;

	const BasicCamera* m_camera;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
