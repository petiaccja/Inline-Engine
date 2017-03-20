#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../PerspectiveCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../GraphicsContext.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

namespace inl::gxeng::nodes {


class DepthPrepass :
	virtual public GraphicsNode,
	// depth texture, entities, camera
	virtual public exc::InputPortConfig<pipeline::Texture2D, const EntityCollection<MeshEntity>*, const BasicCamera*>,
	virtual public exc::OutputPortConfig<pipeline::Texture2D>
{
public:
	DepthPrepass(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;
	
protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_transformBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void RenderScene(
		const DepthStencilView2D& dsv,
		const EntityCollection<MeshEntity>& entities,
		const BasicCamera* camera,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes

