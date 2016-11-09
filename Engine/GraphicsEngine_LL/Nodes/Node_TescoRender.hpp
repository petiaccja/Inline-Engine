#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4250) // inheritence via dominance bug
#endif 

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../ConstBufferHeap.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl {
namespace gxeng {
namespace nodes {


class TescoRender :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<RenderTargetView, DepthStencilView, const Camera*, const EntityCollection<MeshEntity>*>,
	virtual public exc::OutputPortConfig<RenderTargetView>
{
public:
	TescoRender(gxapi::IGraphicsApi* graphicsApi, gxapi::IGxapiManager* gxapiManager);

	void Update() override {}

	void Notify(exc::InputPortBase* sender) override {}

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context) {
			ExecutionResult result;

			auto target = this->GetInput<0>().Get();
			this->GetInput<0>().Clear();

			auto depthStencil = this->GetInput<1>().Get();
			this->GetInput<1>().Clear();

			const Camera* camera = this->GetInput<2>().Get();
			this->GetInput<2>().Clear();

			const EntityCollection<MeshEntity>* entities = this->GetInput<3>().Get();
			this->GetInput<3>().Clear();

			this->GetOutput<0>().Set(target);

			bool rtvIsValid = target.GetResource().get() != nullptr;
			if (rtvIsValid && entities) {
				GraphicsCommandList cmdList = context.GetGraphicsCommandList();
				RenderScene(target, depthStencil, camera, *entities, cmdList);
				result.AddCommandList(std::move(cmdList));
			}

			return result;
		} });
	}

protected:
	Binder m_binder;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	BindParameter m_cbBindParam;
	BindParameter m_texBindParam;

private:
	void RenderScene(RenderTargetView& rtv, DepthStencilView& dsv, const Camera* camera, const EntityCollection<MeshEntity>& entities, GraphicsCommandList& commandList);
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
