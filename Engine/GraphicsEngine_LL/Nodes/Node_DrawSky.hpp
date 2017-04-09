#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../BasicCamera.hpp"
#include "../ConstBufferHeap.hpp"
#include "../DirectionalLight.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <optional>

namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: frame color, frame depth stencil, camera, sun
/// Output: frame color
/// </summary>
class DrawSky :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D, Texture2D, const BasicCamera*, const EntityCollection<DirectionalLight>*>,
	virtual public exc::OutputPortConfig<Texture2D>
{
public:
	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	
	void Initialize(EngineContext& context) override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;
	bool fsqInited;

private: // execution
	RenderTargetView2D m_rtv;
	DepthStencilView2D m_dsv;
	const BasicCamera* m_camera;
	const EntityCollection<DirectionalLight>* m_suns;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_sunCbBindParam;
	BindParameter m_camCbBindParam;

	gxeng::ShaderProgram m_shader;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	gxapi::eFormat m_colorFormat = gxapi::eFormat::UNKNOWN;
	gxapi::eFormat m_depthStencilFormat = gxapi::eFormat::UNKNOWN;
};


} // namespace inl::gxeng::nodes
