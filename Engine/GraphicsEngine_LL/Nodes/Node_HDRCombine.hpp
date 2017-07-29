#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../BasicCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"
#include "../Image.hpp"

#include <optional>

namespace inl::gxeng::nodes {


class HDRCombine :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, Texture2D, Texture2D, Texture2D, inl::gxeng::Image*, inl::gxeng::Image*, inl::gxeng::Image*, const BasicCamera*>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	HDRCombine();

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
	BindParameter m_luminanceTexBindParam;
	BindParameter m_bloomTexBindParam;
	BindParameter m_uniformsBindParam;
	BindParameter m_colorGradingTexBindParam;
	BindParameter m_lensFlareTexBindParam;
	BindParameter m_lensFlareDirtTexBindParam;
	BindParameter m_lensFlareStarTexBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RenderTargetView2D m_combine_rtv;

	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;
	bool fsqInited;


protected: // render context
	TextureView2D m_inputTexSrv;
	TextureView2D m_luminanceTexSrv;
	TextureView2D m_bloomTexSrv;
	TextureView2D m_lensFlareTexSrv;

	const BasicCamera* m_camera;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

