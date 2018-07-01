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
	virtual public InputPortConfig<Texture2D, Texture2D, Texture2D, Texture2D, Texture2D>,
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
	BindParameter m_uniformsBindParam;
	ShaderProgram m_minfilterShader;
	std::unique_ptr<gxapi::IPipelineState> m_minfilterPSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	std::vector<TextureView2D> m_inputTex_srv;
	std::vector<RenderTargetView2D> m_filterHorizontal_rtv;
	std::vector<TextureView2D> m_filterHorizontal_srv;
	std::vector<RenderTargetView2D> m_filterVertical_rtv;
	std::vector<TextureViewCube> m_cubeMinfilter_srv;
	std::vector<TextureView2D> m_csmMinfilter_srv;

protected: // render context
	TextureView2D m_csmTexSrv;
	TextureView2D m_shadowMxTexSrv;
	TextureView2D m_csmSplitsTexSrv;
	TextureView2D m_lightMvpTexSrv;
	TextureViewCube m_cubeShadowTexSrv;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

