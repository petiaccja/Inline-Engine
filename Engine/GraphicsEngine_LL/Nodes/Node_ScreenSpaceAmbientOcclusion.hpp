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


class ScreenSpaceAmbientOcclusion :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, const BasicCamera*, Texture2D>,
	virtual public OutputPortConfig<Texture2D>
{
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
	std::optional<Binder> m_binder;
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
	RenderTargetView2D m_ssao_rtv;
	RenderTargetView2D m_blur_horizontal_rtv;
	RenderTargetView2D m_blur_vertical0_rtv;
	RenderTargetView2D m_blur_vertical1_rtv;
	TextureView2D m_ssao_srv;
	TextureView2D m_blur_horizontal_srv;
	TextureView2D m_blur_vertical0_srv;
	TextureView2D m_blur_vertical1_srv;

	Mat44 m_prevVP;

protected: // render context
	TextureView2D m_depthTexSrv;
	const BasicCamera* m_camera;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

