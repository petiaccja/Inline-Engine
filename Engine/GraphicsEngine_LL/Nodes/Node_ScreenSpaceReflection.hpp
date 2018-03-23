#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../BasicCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <optional>

namespace inl::gxeng::nodes {


class ScreenSpaceReflection :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, Texture2D, const BasicCamera*>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	static const char* Info_GetName() { return "ScreenSpaceReflection"; }
	ScreenSpaceReflection();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_inputTexBindParam;
	BindParameter m_depthTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	ShaderProgram m_downsampleShader;
	ShaderProgram m_blurShader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	std::vector<std::unique_ptr<gxapi::IPipelineState> > m_downsamplePSO;
	std::vector<std::unique_ptr<gxapi::IPipelineState> > m_blurHorizontalPSO;
	std::vector<std::unique_ptr<gxapi::IPipelineState> > m_blurVerticalPSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_ssr_rtv;

protected: // render context
	TextureView2D m_inputTexSrv;
	std::vector<RenderTargetView2D> m_input_rtv;
	std::vector<TextureView2D> m_input_srv;
	std::vector<RenderTargetView2D> m_blur_rtv;
	std::vector<TextureView2D> m_blur_srv;
	TextureView2D m_depthTexSrv;
	const BasicCamera* m_camera;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

