#pragma once

#include "../Helpers/PipelineStateCache.hpp"

#include <GraphicsEngine/Scene/IDirectionalLight.hpp>
#include <GraphicsEngine/Scene/IHeightmapEntity.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {


class RenderForwardHeightmaps : virtual public GraphicsNode,
							public GraphicsTask,
							public InputPortConfig<Texture2D, Texture2D, const BasicCamera*, const EntityCollection<IHeightmapEntity>*, const EntityCollection<IDirectionalLight>*>,
							public OutputPortConfig<Texture2D, Texture2D> {
public:
	static const char* Info_GetName() { return "RenderForwardHeightmaps"; }

	RenderForwardHeightmaps();

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

private:
	RenderTargetView2D m_rtv;
	DepthStencilView2D m_dsv;

	PipelineStateCache m_psoCache;
};


} // namespace inl::gxeng::nodes
