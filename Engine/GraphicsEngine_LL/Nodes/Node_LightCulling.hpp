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
	virtual public exc::InputPortConfig<Texture2D, const BasicCamera*>,// const EntityCollection<PointLight>*>,
	virtual public exc::OutputPortConfig<Texture2D>
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
	std::optional<Binder> m_binder;
	BindParameter m_inputBindParam;
	BindParameter m_outputBindParam;
	BindParameter m_uniformsBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_CSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RWTextureView2D m_lightCullDataUAV;

protected: // render context
	TextureView2D m_depthTexSrv;
	const BasicCamera* m_camera;
	//const EntityCollection<PointLight>* m_lights;

private:
	uint64_t m_width, m_height;
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

