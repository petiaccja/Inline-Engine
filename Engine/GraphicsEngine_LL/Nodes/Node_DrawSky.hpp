#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../ConstBufferHeap.hpp"
#include "../TextureViewPack.hpp"
#include "../DirectionalLight.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl::gxeng::nodes {


class DrawSky :
	virtual public GraphicsNode,
	// Inputs: HDR image from CombineGBuffer, depth stencil, camera, sun
	virtual public exc::InputPortConfig<RenderTargetPack, DepthStencilPack, const Camera*, const DirectionalLight*>,
	virtual public exc::OutputPortConfig<RenderTargetPack>
{
public:
	DrawSky(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context) {
			ExecutionResult result;

			auto renderTarget = this->GetInput<0>().Get();
			this->GetInput<0>().Clear();

			auto depthStencil = this->GetInput<1>().Get();
			this->GetInput<1>().Clear();

			const Camera* camera = this->GetInput<2>().Get();
			this->GetInput<2>().Clear();

			const DirectionalLight* sun = this->GetInput<3>().Get();
			this->GetInput<3>().Clear();

			GraphicsCommandList cmdList = context.GetGraphicsCommandList();
			Render(renderTarget.rtv, depthStencil.dsv, camera, sun, cmdList);
			result.AddCommandList(std::move(cmdList));

			this->GetOutput<0>().Set(renderTarget);

			return result;
		} });
	}

protected:
	GraphicsContext m_graphicsContext;
	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;

protected:
	Binder m_binder;
	BindParameter m_cbBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void Render(
		RenderTargetView2D& rtv,
		DepthStencilView2D& dsv,
		const Camera* camera,
		const DirectionalLight* sun,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes
