#pragma once

#include "../GraphicsNode.hpp"
#pragma once 

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
/// Inputs: render target, scene objects, light cascade MVP transform matrices in a texture
/// Output: render target
/// </summary>
class CSM :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D, const EntityCollection<MeshEntity>*, Texture2D>,
	virtual public exc::OutputPortConfig<Texture2D>
{
public:
	CSM();

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_uniformsBindParam;
	BindParameter m_lightMVPBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;
	gxapi::eFormat m_depthStencilFormat;

private: // render context
	std::vector<DepthStencilView2D> m_dsvs;
	const EntityCollection<MeshEntity>* m_entities;
	TextureView2D m_lightMVPTexSrv;

private:
	void InitRenderTarget(unsigned width, unsigned height);
	void RenderScene(
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes

