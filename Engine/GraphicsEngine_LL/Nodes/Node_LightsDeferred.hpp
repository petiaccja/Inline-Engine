#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../ConstBufferHeap.hpp"
#include "../TextureViewPack.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl::gxeng::nodes {


class LightsDeferred :
	virtual public GraphicsNode,
	// Inputs: HDR image from CombineGBuffer, depth, albedoRoughness, normal, camera, (lights TODO)
	virtual public exc::InputPortConfig<RenderTargetPack, DepthStencilPack, RenderTargetPack, RenderTargetPack, const Camera*>,
	virtual public exc::OutputPortConfig<RenderTargetPack>
{
public:
	LightsDeferred(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext&) override {}

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context) {
			ExecutionResult result;

			auto renderTarget = this->GetInput<0>().Get();
			this->GetInput<0>().Clear();

			auto depthStencil = this->GetInput<1>().Get();
			this->GetInput<1>().Clear();

			const Camera* camera = this->GetInput<4>().Get();
			this->GetInput<4>().Clear();

			// TODO
			/*
			const EntityCollection<PointLight>* lights = this->GetInput<5>().Get();
			this->GetInput<5>().Clear();

			if (lights) {
				GraphicsCommandList cmdList = context.GetGraphicsCommandList();
				RenderScene(renderTarget.rtv, depthStencil, camera, *entities, cmdList);
				result.AddCommandList(std::move(cmdList));
			}
			*/

			this->GetOutput<0>().Set(renderTarget);

			return result;
		} });
	}

protected:
	RenderTargetPack m_renderTarget;

protected:
	Binder m_binder;
	BindParameter m_cbBindParam;
	BindParameter m_texBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void RenderScene(
		RenderTargetView& rtv,
		Texture2DSRV& depthStencil,
		Texture2DSRV& albedoRoughness,
		Texture2DSRV& normal,
		const Camera* camera,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes
