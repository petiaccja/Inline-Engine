#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

namespace inl::gxeng::nodes {


class DOFMain : virtual public GraphicsNode,
				virtual public GraphicsTask,
				virtual public InputPortConfig<Texture2D, Texture2D, Texture2D, const BasicCamera*, Texture2D, Texture2D>,
				virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "DOFMain"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	DOFMain();

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
	BindParameter m_neighborhoodMaxTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_postfilterShader;
	ShaderProgram m_upsampleShader;
	ShaderProgram m_mainShader;
	std::unique_ptr<gxapi::IPipelineState> m_postfilterPSO;
	std::unique_ptr<gxapi::IPipelineState> m_mainPSO;
	std::unique_ptr<gxapi::IPipelineState> m_upsamplePSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_postfilterRTV;
	RenderTargetView2D m_mainRTV;
	RenderTargetView2D m_upsampleRTV;
	TextureView2D m_mainSRV;
	TextureView2D m_postfilterSRV;

protected: // render context
	TextureView2D m_inputTexSrv;
	TextureView2D m_halfDepthTexSrv;
	TextureView2D m_depthTexSrv;
	TextureView2D m_neighborhoodMaxTexSrv;
	TextureView2D m_originalTexSrv;
	const BasicCamera* m_camera;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
