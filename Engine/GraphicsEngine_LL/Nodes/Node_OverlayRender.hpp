#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../BasicCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../GraphicsContext.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

namespace inl::gxeng::nodes {

class OverlayRender :
	virtual public GraphicsNode,

	virtual public exc::InputPortConfig<const EntityCollection<OverlayEntity>*, const BasicCamera*>,
	virtual public exc::OutputPortConfig<pipeline::Texture2D>
{
public:
	OverlayRender(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;


protected:
	struct BasePipelineObjects {
		Binder binder;
		BindParameter transformParam;
		std::unique_ptr<gxapi::IPipelineState> pso;
	};
	struct TexturedPipelineObjects : public BasePipelineObjects {
		BindParameter textureParam;
	};
	struct ColoredPipelineObjects : public BasePipelineObjects {
		BindParameter colorParam;
	};


protected:
	void InitColoredBindings(gxapi::IGraphicsApi * graphicsApi);
	void InitTexturedBindings(gxapi::IGraphicsApi * graphicsApi);

	gxapi::GraphicsPipelineStateDesc GetPsoDesc(
		std::vector<gxapi::InputElementDesc>& inputElementDesc,
		gxeng::ShaderProgram& shader,
		const Binder& binder) const;

	void InitColoredPso();
	void InitTexturedPso();

	void InitRenderTarget(unsigned width, unsigned height);
	void RenderScene(
		const EntityCollection<OverlayEntity>& entities,
		const BasicCamera* camera,
		GraphicsCommandList& commandList);

	static bool CheckMeshFormat(Mesh* mesh);

protected:
	static constexpr auto COLOR_FORMAT = gxapi::eFormat::R8G8B8A8_UNORM;

protected:
	RenderTargetView2D m_rtv;
	TextureView2D m_renderTargetSrv;

	GraphicsContext m_graphicsContext;

	ColoredPipelineObjects m_coloredPipeline;
	TexturedPipelineObjects m_texturedPipeline;
};


} // namespace inl::gxeng::nodes

