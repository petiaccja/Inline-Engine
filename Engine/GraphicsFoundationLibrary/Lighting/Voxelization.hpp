#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>


namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: entities, camera
/// Voxelizes scene into a dense 3D texture
/// </summary>
class Voxelization : virtual public GraphicsNode,
					 virtual public GraphicsTask,
					 virtual public InputPortConfig<const EntityCollection<MeshEntity>*>,
					 virtual public OutputPortConfig<Texture3D, Texture3D> {
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
	Binder m_binder;
	BindParameter m_uniformsBindParam;
	BindParameter m_albedoTexBindParam;
	BindParameter m_voxelColorTexBindParam;
	BindParameter m_voxelAlphaNormalTexBindParam;
	ShaderProgram m_voxelizationShader;
	ShaderProgram m_mipmapShader;
	std::unique_ptr<gxapi::IPipelineState> m_voxelizationPSO;
	std::unique_ptr<gxapi::IPipelineState> m_mipmapCSO;

	bool m_outputTexturesInited = false;
	std::vector<RWTextureView3D> m_voxelColorTexUAV;
	TextureView3D m_voxelColorTexSRV;
	std::vector<TextureView3D> m_voxelColorTexMipSRV;
	std::vector<RWTextureView3D> m_voxelAlphaNormalTexUAV;
	TextureView3D m_voxelAlphaNormalTexSRV;
	std::vector<TextureView3D> m_voxelAlphaNormalTexMipSRV;

private: // execution context
	const EntityCollection<MeshEntity>* m_entities;

	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
