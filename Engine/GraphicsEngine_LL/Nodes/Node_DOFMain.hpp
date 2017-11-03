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


class DOFMain :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, Texture2D, Texture2D, const BasicCamera*, Texture2D, Texture2D>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	DOFMain();

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
	BindParameter m_neighborhoodMaxTexBindParam;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_postfilter_shader;
	ShaderProgram m_upsample_shader;
	ShaderProgram m_main_shader;
	std::unique_ptr<gxapi::IPipelineState> m_postfilter_PSO;
	std::unique_ptr<gxapi::IPipelineState> m_main_PSO;
	std::unique_ptr<gxapi::IPipelineState> m_upsample_PSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_postfilter_rtv;
	RenderTargetView2D m_main_rtv;
	RenderTargetView2D m_upsample_rtv;
	TextureView2D m_main_srv;
	TextureView2D m_postfilter_srv;

	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;
	bool fsqInited;


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

