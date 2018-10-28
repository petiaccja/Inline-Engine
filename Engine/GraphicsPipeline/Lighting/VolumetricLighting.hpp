#pragma once

#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsEngine_LL/BasicCamera.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>

namespace inl::gxeng::nodes {


class VolumetricLighting : virtual public GraphicsNode,
						   virtual public GraphicsTask,
						   virtual public InputPortConfig<Texture2D, Texture2D, Texture2D, const BasicCamera*, Texture2D, Texture2D, Texture2D>,
						   virtual public OutputPortConfig<Texture2D> {
public:
	static const char* Info_GetName() { return "VolumetricLighting"; }
	const std::string& GetInputName(size_t index) const override;
	const std::string& GetOutputName(size_t index) const override;
	VolumetricLighting();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	Binder m_binder;
	BindParameter m_inputColorBindParam;
	BindParameter m_dstBindParam;
	BindParameter m_volDst0BindParam;
	BindParameter m_volDst1BindParam;
	BindParameter m_depthBindParam;
	BindParameter m_cullBindParam;
	BindParameter m_cullRoBindParam;
	BindParameter m_lightCullBindParam;
	BindParameter m_uniformsBindParam;
	BindParameter m_csmTexBindParam;
	BindParameter m_shadowMxTexBindParam;
	BindParameter m_csmSplitsTexBindParam;
	BindParameter m_lightMvpTexBindParam;
	ShaderProgram m_sdfCullingShader;
	ShaderProgram m_volumetricLightingShader;
	std::unique_ptr<gxapi::IPipelineState> m_sdfCullingCSO;
	std::unique_ptr<gxapi::IPipelineState> m_volumetricLightingCSO;

protected: // outputs
	bool m_outputTexturesInited = false;
	RWTextureView2D m_sdfCullDataUAV;
	TextureView2D m_sdfCullDataSRV;
	TextureView2D m_lightCullDataSRV;
	TextureView2D m_colorTexSRV;
	TextureView2D m_shadowMXTexSRV;
	TextureView2D m_csmSplitsTexSRV;
	TextureView2D m_csmTexSRV;
	RWTextureView2D m_volDstTexUAV[2];
	RWTextureView2D m_dstTexUAV;

protected: // render context
	TextureView2D m_depthTexSrv;
	const BasicCamera* m_camera;
	Mat44 m_prevVP;
	//const EntityCollection<PointLight>* m_lights;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes
