#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../GraphicsContext.hpp"
#include "../TextureViewPack.hpp"
#include "../WindowResizeListener.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl::gxeng::nodes {


class CombineGBuffer :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<DepthStencilPack, RenderTargetPack, RenderTargetPack, const Camera*, const DirectionalLight*>,
	virtual public exc::OutputPortConfig<RenderTargetPack>,
	public WindowResizeListener
{
public:
	CombineGBuffer(gxapi::IGraphicsApi* graphicsApi, unsigned width, unsigned height);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context) {
			ExecutionResult result;

			auto depthStencil = this->GetInput<0>().Get();
			this->GetInput<0>().Clear();

			auto albedoRoughness = this->GetInput<1>().Get();
			this->GetInput<1>().Clear();

			auto normal = this->GetInput<2>().Get();
			this->GetInput<2>().Clear();

			const Camera* camera = this->GetInput<3>().Get();
			this->GetInput<3>().Clear();

			const DirectionalLight* sun = this->GetInput<4>().Get();
			this->GetInput<4>().Clear();

			GraphicsCommandList cmdList = context.GetGraphicsCommandList();
			RenderCombined(depthStencil.srv, albedoRoughness.srv, normal.srv, camera, sun, cmdList);
			result.AddCommandList(std::move(cmdList));

			this->GetOutput<0>().Set(m_renderTarget);

			return result;
		} });
	}

	void WindowResized(unsigned width, unsigned height) override;

protected:
	unsigned m_width;
	unsigned m_height;
	RenderTargetPack m_renderTarget;
	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_sunBindParam;
	BindParameter m_albedoRoughnessBindParam;
	BindParameter m_normalBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void InitBuffer();
	void RenderCombined(
		TextureView2D& depthStencil,
		TextureView2D& albedoRoughness,
		TextureView2D& normal,
		const Camera* camera,
		const DirectionalLight* sun,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes
