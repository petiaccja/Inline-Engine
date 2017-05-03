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


class LightCulling :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D, const BasicCamera*, const EntityCollection<DirectionalLight>*>,
	virtual public exc::OutputPortConfig<Texture2D, Texture2D, Texture2D>
{
public:
	LightCulling();

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}

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
	BindParameter m_reductionBindParam;
	BindParameter m_outputBindParam0;
	BindParameter m_outputBindParam1;
	BindParameter m_outputBindParam2;
	BindParameter m_uniformsBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_CSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RWTextureView2D m_light_mvp_uav;
	RWTextureView2D m_shadow_mx_uav;
	RWTextureView2D m_csm_splits_uav;

protected: // render context
	TextureView2D m_reductionTexSrv;
	const BasicCamera* m_camera;
	const EntityCollection<DirectionalLight>* m_suns;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

