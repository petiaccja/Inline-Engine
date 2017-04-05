#pragma once

#include "../GraphicsNode.hpp"


#include "../Scene.hpp"
#include "../PerspectiveCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

namespace inl::gxeng::nodes {


class DepthReduction :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D>,
	virtual public exc::OutputPortConfig<Texture2D>
{
public:
	DepthReduction(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void Initialize(EngineContext& context) override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	TextureView2D m_depthView;

	unsigned m_width;
	unsigned m_height;

	gxeng::RWTextureView2D m_uav;
	gxeng::TextureView2D m_srv;

protected:
	Binder m_binder;
	BindParameter m_depthBindParam;
	BindParameter m_outputBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_CSO;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

