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


class ShadowFilter :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, Texture2D, Texture2D, Texture2D, Texture2D, Texture2D, const BasicCamera*>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	static const char* Info_GetName() { return "ShadowFilter"; }
	ShadowFilter();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_inputTexBindParam;
	BindParameter m_csmTexBindParam;
	BindParameter m_shadowMxTexBindParam;
	BindParameter m_csmSplitsTexBindParam;
	BindParameter m_lightMvpTexBindParam;
	BindParameter m_cubeShadowTexBindParam;
	BindParameter m_csmMinfilterTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_minfilterShader;
	ShaderProgram m_penumbraShader;
	std::unique_ptr<gxapi::IPipelineState> m_minfilterPSO;
	std::unique_ptr<gxapi::IPipelineState> m_penumbraPSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	std::vector<TextureView2D> m_inputTex_srv;
	std::vector<RenderTargetView2D> m_filterHorizontal_rtv;
	std::vector<TextureView2D> m_filterHorizontal_srv;
	std::vector<RenderTargetView2D> m_filterVertical_rtv;
	std::vector<TextureViewCube> m_cubeMinfilter_srv;
	std::vector<TextureView2D> m_csmMinfilter_srv;

	RenderTargetView2D m_shadowLayers_rtv;
	RenderTargetView2D m_penumbraLayers_rtv;

protected: // render context
	TextureView2D m_csmTexSrv;
	TextureView2D m_shadowMxTexSrv;
	TextureView2D m_csmSplitsTexSrv;
	TextureView2D m_lightMvpTexSrv;
	TextureView2D m_depthTexSrv;
	TextureViewCube m_cubeShadowTexSrv;

	const BasicCamera* m_camera;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

