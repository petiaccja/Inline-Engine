#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4250) // inheritence via dominance bug
#endif 

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl {
namespace gxeng {
namespace nodes {


class TescoRender :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<BackBuffer*, const EntityCollection<MeshEntity>*>,
	virtual public exc::OutputPortConfig<BackBuffer*>
{
public:
	TescoRender(gxapi::IGraphicsApi* graphicsApi, gxapi::IGxapiManager* gxapiManager);

	void Update() override {}

	void Notify(exc::InputPortBase* sender) override {}

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context) {
			ExecutionResult result;

			BackBuffer* target = this->GetInput<0>().Get();
			this->GetInput<0>().Clear();
			const EntityCollection<MeshEntity>* entities = this->GetInput<1>().Get();
			this->GetInput<1>().Clear();
			this->GetOutput<0>().Set(target);

			if (target && entities) {
				GraphicsCommandList cmdList = context.GetGraphicsCommandList();
				RenderScene(target, *entities, cmdList);
				result.AddCommandList(std::move(cmdList));
			}

			return result;
		} });
	}

protected:
	Binder m_binder;
	std::unique_ptr<gxapi::IRootSignature> m_rootSignature;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void RenderScene(BackBuffer* target, const EntityCollection<MeshEntity>& entities, GraphicsCommandList& commandList);
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
