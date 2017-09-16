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
	virtual public InputPortConfig<const EntityCollection<MeshEntity>*, const BasicCamera*, Texture2D, Texture2D>,
	virtual public OutputPortConfig<Texture3D, Texture2D, Texture2D>
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
	BindParameter m_voxelTexBindParam;
	ShaderProgram m_shader;
	ShaderProgram m_visualizerShader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	std::unique_ptr<gxapi::IPipelineState> m_visualizerPSO;

	bool m_outputTexturesInited = false;
	RWTextureView3D m_voxelTexUAV;
	TextureView3D m_voxelTexSRV;

	RenderTargetView2D m_visualizationTexRTV;
	DepthStencilView2D m_visualizationDSV;

private: // execution context
	const EntityCollection<MeshEntity>* m_entities;
	const BasicCamera* m_camera;

	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

