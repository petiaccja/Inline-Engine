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
	virtual public InputPortConfig<const EntityCollection<MeshEntity>*>,
	virtual public OutputPortConfig<Texture3D, Texture3D>
{
public:
	static const char* Info_GetName() { return "Voxelization"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
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
	BindParameter m_voxelSecondaryTexReadBindParam;
	ShaderProgram m_shader;
	ShaderProgram m_mipmapShader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	std::unique_ptr<gxapi::IPipelineState> m_mipmapCSO;

	bool m_outputTexturesInited = false;
	std::vector<RWTextureView3D> m_voxelTexUAV;
	TextureView3D m_voxelTexSRV;
	std::vector<TextureView3D> m_voxelTexMipSRV;
	std::vector<RWTextureView3D> m_voxelSecondaryTexUAV;
	TextureView3D m_voxelSecondaryTexSRV;
	std::vector<TextureView3D> m_voxelSecondaryTexMipSRV;

private: // execution context
	const EntityCollection<MeshEntity>* m_entities;

	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

