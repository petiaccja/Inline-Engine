#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../PerspectiveCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <optional>

namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: render target, entities, camera
/// </summary>
class DepthPrepass :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, const EntityCollection<MeshEntity>*, const BasicCamera*>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	DepthPrepass();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;
	
protected:
	std::optional<Binder> m_binder;
	BindParameter m_transformBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	gxapi::eFormat m_depthStencilFormat = gxapi::eFormat::UNKNOWN;

private: // execution context
	DepthStencilView2D m_targetDsv;
	const EntityCollection<MeshEntity>* m_entities;
	const BasicCamera* m_camera;
};


} // namespace inl::gxeng::nodes

