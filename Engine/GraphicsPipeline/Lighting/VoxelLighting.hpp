#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: entities, camera
/// Voxelizes scene into a dense 3D texture
/// </summary>
class VoxelLighting : virtual public GraphicsNode,
					  virtual public GraphicsTask,
					  virtual public InputPortConfig<const BasicCamera*, Texture3D, Texture3D, Texture2D, Texture2D, Texture2D, Texture2D, Texture2D, Texture2D, Texture2D>,
					  virtual public OutputPortConfig<Texture2D, Texture2D> {
public:
	static const char* Info_GetName() { return "VoxelLighting"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	VoxelLighting();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_uniformsBindParam;
	BindParameter m_voxelColorTexBindParam;
	BindParameter m_voxelLightTexBindParam;
	BindParameter m_tex0BindParam;
	BindParameter m_tex1BindParam;
	BindParameter m_tex2BindParam;
	BindParameter m_tex3BindParam;
	BindParameter m_tex4BindParam;
	BindParameter m_tex5BindParam;
	BindParameter m_tex6BindParam;
	ShaderProgram m_visualizerShader;
	ShaderProgram m_finalGatherShader;
	ShaderProgram m_lightInjectionCSMShader;
	ShaderProgram m_mipmapShader;
	std::unique_ptr<gxapi::IPipelineState> m_visualizerPSO;
	std::unique_ptr<gxapi::IPipelineState> m_finalGatherPSO;
	std::unique_ptr<gxapi::IPipelineState> m_lightInjectionCSMPSO;
	std::unique_ptr<gxapi::IPipelineState> m_mipmapCSO;

	bool m_outputTexturesInited = false;
	TextureView3D m_voxelColorTexSRV;
	TextureView3D m_voxelAlphaNormalTexSRV;

	std::vector<RWTextureView3D> m_voxelLightTexUAV;
	TextureView3D m_voxelLightTexSRV;
	std::vector<TextureView3D> m_voxelLightTexMipSRV;

	TextureView2D m_shadowCSMTexSrv;
	TextureView2D m_shadowCSMExtentsTexSrv;
	TextureView2D m_velocityNormalTexSrv;
	TextureView2D m_albedoRoughnessMetalnessTexSrv;
	TextureView2D m_screenSpaceAmbientOcclusionTexSrv;

	RenderTargetView2D m_visualizationTexRTV;
	DepthStencilView2D m_visualizationDSV;
	TextureView2D m_depthTexSRV;

private: // execution context
	const BasicCamera* m_camera;

	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
