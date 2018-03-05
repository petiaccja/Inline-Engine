#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../PerspectiveCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "../Material.hpp"
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
	virtual public InputPortConfig<const EntityCollection<MeshEntity>*, const BasicCamera*, Texture2D, Texture2D, Texture2D, Texture2D, Texture2D>,
	virtual public OutputPortConfig<Texture3D, Texture2D, Texture2D>
{
public:
	static const char* Info_GetName() { return "Voxelization"; }
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
	BindParameter m_albedoTexBindParam;
	BindParameter m_voxelTexBindParam;
	BindParameter m_voxelSecondaryTexBindParam;
	BindParameter m_voxelLightTexBindParam;
	BindParameter m_shadowCSMTexBindParam;
	BindParameter m_shadowCSMExtentsTexBindParam;
	BindParameter m_voxelSecondaryTexReadBindParam;
	ShaderProgram m_shader;
	ShaderProgram m_visualizerShader;
	ShaderProgram m_finalGatherShader;
	ShaderProgram m_lightInjectionCSMShader;
	ShaderProgram m_mipmapShader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	std::unique_ptr<gxapi::IPipelineState> m_visualizerPSO;
	std::unique_ptr<gxapi::IPipelineState> m_finalGatherPSO;
	std::unique_ptr<gxapi::IPipelineState> m_lightInjectionCSMPSO;
	std::unique_ptr<gxapi::IPipelineState> m_mipmapCSO;

	bool m_outputTexturesInited = false;
	std::vector<RWTextureView3D> m_voxelTexUAV;
	TextureView3D m_voxelTexSRV;
	std::vector<RWTextureView3D> m_voxelSecondaryTexUAV;
	TextureView3D m_voxelSecondaryTexSRV;
	std::vector<RWTextureView3D> m_voxelLightTexUAV;
	TextureView3D m_voxelLightTexSRV;

	TextureView2D m_shadowCSMTexSrv;
	TextureView2D m_shadowCSMExtentsTexSrv;
	TextureView2D m_normalTexSrv;

	RenderTargetView2D m_visualizationTexRTV;
	DepthStencilView2D m_visualizationDSV;
	TextureView2D m_depthTexSRV;

	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;
	bool fsqInited;

private: // execution context
	const EntityCollection<MeshEntity>* m_entities;
	const BasicCamera* m_camera;

	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

