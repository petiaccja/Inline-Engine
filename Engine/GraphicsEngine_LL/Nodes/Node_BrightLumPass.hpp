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


class BrightLumPass :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D>,
	virtual public OutputPortConfig<Texture2D, Texture2D>
{
public:
	static const char* Info_GetName() { return "BrightLumPass"; }
	BrightLumPass();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	//gxeng::RWTextureView2D m_light_mvp_uav;
	//gxeng::TextureView2D m_light_mvp_srv;
	//
	//gxeng::RWTextureView2D m_shadow_mx_uav;
	//gxeng::TextureView2D m_shadow_mx_srv;
	//
	//gxeng::RWTextureView2D m_csm_splits_uav;
	//gxeng::TextureView2D m_csm_splits_srv;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_inputTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_bright_pass_rtv;
	RenderTargetView2D m_luminance_rtv;

protected: // render context
	TextureView2D m_inputTexSrv;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

