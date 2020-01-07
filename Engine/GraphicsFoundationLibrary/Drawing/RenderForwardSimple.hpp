#pragma once

#include "../Helpers/PipelineStateCache.hpp"

#include <GraphicsEngine/Scene/IDirectionalLight.hpp>
#include <GraphicsEngine/Scene/IMeshEntity.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


class RenderForwardSimple : virtual public GraphicsNode,
							public GraphicsTask,
							public InputPortConfig<Texture2D, Texture2D, const BasicCamera*, const EntityCollection<IMeshEntity>*, const EntityCollection<IDirectionalLight>*>,
							public OutputPortConfig<Texture2D, Texture2D> {
public:
	static const char* Info_GetName() { return "RenderForwardSimple"; }

	RenderForwardSimple();

	void Initialize(EngineContext& context) override {
		SetTaskSingle(this);
	}

	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

	// Methods not used.
	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;

private:
	void CreateRenderTargetViews(SetupContext& context, const Texture2D& rt, const Texture2D& ds);
	void UpdatePsoCache(const Texture2D& renderTarget, const Texture2D& depthTarget);
	void RenderEntities(RenderContext& context, GraphicsCommandList& commandList);

private:
	RenderTargetView2D m_rtv;
	DepthStencilView2D m_dsv;

	PipelineStateCache m_psoCache;
};


} // namespace inl::gxeng::nodes
