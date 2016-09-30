#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4250) // inheritence via dominance bug
#endif 

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "GraphicsApi_D3D12/CommandList.hpp"


namespace inl {
namespace gxeng {
namespace nodes {


class TescoRender :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<Texture2D*, const EntityCollection<MeshEntity>*>,
	virtual public exc::OutputPortConfig<Texture2D*>
{
public:
	TescoRender() {
		this->GetInput<0>().Set(nullptr);
		this->GetInput<1>().Set(nullptr);
	}

	void Update() override {}

	void Notify(exc::InputPortBase* sender) override {}

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context) {
			ExecutionResult result;

			Texture2D* target = this->GetInput<0>().Get();
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

private:
	void RenderScene(Texture2D* target, const EntityCollection<MeshEntity>& entities, GraphicsCommandList& commandList);
};



} // namespace nodes
} // namespace gxeng
} // namespace inl
