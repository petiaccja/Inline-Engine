#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../BasicCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <optional>

namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: framebuffer, overlay entites, overlay camera
/// </summary>
class OverlayRender :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D, const EntityCollection<OverlayEntity>*, const BasicCamera*>,
	virtual public exc::OutputPortConfig<Texture2D>
{
public:
	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;


protected:
	struct BasePipelineObjects {
		std::optional<Binder> binder;
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
	gxapi::eFormat m_renderTargetFormat;

	ShaderProgram m_coloredShader;
	ColoredPipelineObjects m_coloredPipeline;

	ShaderProgram m_texturedShader;
	TexturedPipelineObjects m_texturedPipeline;

protected:
	void InitColoredBindings(SetupContext& context);
	void InitTexturedBindings(SetupContext& context);

	gxapi::GraphicsPipelineStateDesc GetPsoDesc(
		std::vector<gxapi::InputElementDesc>& inputElementDesc,
		gxeng::ShaderProgram& shader,
		const Binder& binder,
		gxapi::eFormat renderTargetFormat) const;

	void InitColoredPso(SetupContext& context, gxapi::eFormat renderTargetFormat);
	void InitTexturedPso(SetupContext& context, gxapi::eFormat renderTargetFormat);

	static bool CheckMeshFormat(Mesh* mesh);

private: // execution context
	RenderTargetView2D m_target;
	const EntityCollection<OverlayEntity>* m_entities;
	const BasicCamera* m_camera;
};


} // namespace inl::gxeng::nodes

