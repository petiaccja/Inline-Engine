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
/// Inputs: entities, camera
/// Voxelizes scene into a dense 3D texture
/// </summary>
class Voxelization :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<const EntityCollection<MeshEntity>*, const BasicCamera*>,
	virtual public OutputPortConfig<Texture3D>
{
public:
	Voxelization();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;
	
protected:
	std::optional<Binder> m_binder;
	BindParameter m_uniformsBindParam;
	ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

	bool m_outputTexturesInited = false;
	RWTextureView3D m_voxelTexUAV;
	TextureView3D m_voxelTexSRV;


private: // execution context
	const EntityCollection<MeshEntity>* m_entities;
	const BasicCamera* m_camera;

	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

