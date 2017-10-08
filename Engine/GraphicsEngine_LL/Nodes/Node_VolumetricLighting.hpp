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


class VolumetricLighting :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public InputPortConfig<Texture2D, Texture2D, Texture2D, const BasicCamera*>,
	virtual public OutputPortConfig<Texture2D>
{
public:
	VolumetricLighting();

	void Update() override {}
	void Notify(InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_inputColorBindParam;
	BindParameter m_dstBindParam;
	BindParameter m_depthBindParam;
	BindParameter m_cullBindParam;
	BindParameter m_cullRoBindParam;
	BindParameter m_lightCullBindParam;
	BindParameter m_uniformsBindParam;
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
	RWTextureView2D m_dstTexUAV;

protected: // render context
	TextureView2D m_depthTexSrv;
	const BasicCamera* m_camera;
	//const EntityCollection<PointLight>* m_lights;

private:
	void InitRenderTarget(SetupContext& context);
};


} // namespace inl::gxeng::nodes

