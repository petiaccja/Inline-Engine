#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../ConstBufferHeap.hpp"
#include "../GraphicsContext.hpp"
#include "../TextureViewPack.hpp"
#include "../WindowResizeListener.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl::gxeng::nodes {


class GenGBuffer :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<const Camera*, const EntityCollection<MeshEntity>*>,
	virtual public exc::OutputPortConfig<DepthStencilPack, RenderTargetPack, RenderTargetPack>,
	public WindowResizeListener
{
public:
	GenGBuffer(
		gxapi::IGraphicsApi* graphicsApi,
		unsigned width,
		unsigned height);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext&) override;

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context) {
			ExecutionResult result;

			const Camera* camera = this->GetInput<0>().Get();
			this->GetInput<0>().Clear();

			const EntityCollection<MeshEntity>* entities = this->GetInput<1>().Get();
			this->GetInput<1>().Clear();

			if (entities) {
				GraphicsCommandList cmdList = context.GetGraphicsCommandList();
				RenderScene(camera, *entities, cmdList);
				result.AddCommandList(std::move(cmdList));
			}

			this->GetOutput<0>().Set(m_depthStencil);
			this->GetOutput<1>().Set(m_albedoRoughness);
			this->GetOutput<2>().Set(m_normal);

			return result;
		} });
	}

	void WindowResized(unsigned width, unsigned height) override;

protected:
	unsigned m_width;
	unsigned m_height;

	DepthStencilPack m_depthStencil;

	RenderTargetPack m_albedoRoughness;
	RenderTargetPack m_normal;

	MemoryManager* m_memoryManager;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_cbBindParam;
	BindParameter m_texBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void InitBuffers();
	void RenderScene(const Camera* camera, const EntityCollection<MeshEntity>& entities, GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes
