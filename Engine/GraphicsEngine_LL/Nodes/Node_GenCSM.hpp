#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../ConstBufferHeap.hpp"
#include "../GraphicsContext.hpp"
#include "../TextureViewPack.hpp"
#include "../WindowResizeListener.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <vector>


namespace inl::gxeng::nodes {


/// <summary>
/// Takes a directional light and returns a transformation that acts
/// as a view transform when rendering a shadow map.
/// </summary>
mathfu::Matrix4x4f LightViewTransform(const DirectionalLight* light);

/// <summary>
/// Takes a view transformation of a directional light and 
/// returns a transformation that acts as a projection transform when
/// rendering shadow map for the corresponding directional light, and camera.
/// The frustum of the light is fitted on the camera frustum.
/// </summary>
mathfu::Matrix4x4f LightDirectionalProjectionTransform(const mathfu::Matrix4x4f& lightViewTransform, const Camera* camera);


// A single shadow map and the corresponding camera for cascaded shadow maps
struct ShadowMapCascade {
	Camera subCamera;
	DepthStencilPack map;
};

// Contains shadow maps and their corresponding camera for each cascade of
// a cascaded shadow map
struct ShadowCascades {
	DepthStencilArrayPack mapArray;
	std::vector<Camera> subCameras;
};


// Cascaded shadow mapping
class GenCSM :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<const Camera*, const DirectionalLight*, const EntityCollection<MeshEntity>*>,
	virtual public exc::OutputPortConfig<const ShadowCascades*>
{
public:
	GenCSM(
		gxapi::IGraphicsApi* graphicsApi,
		unsigned width,
		unsigned height);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext&) override;

	Task GetTask() override;

	void SetShadowMapSize(unsigned width, unsigned height);

protected:
	unsigned m_width;
	unsigned m_height;

	MemoryManager* m_memoryManager;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_cbBindParam;
	BindParameter m_texBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

protected:
	unsigned m_cascadeCount = 5;
	ShadowCascades m_cascades;
	//std::vector<ShadowMapCascade> m_cascades; // cameras for subfrusta

private:
	void InitBuffers();
	Camera& CalculateSubCamera(const Camera* camera, unsigned cascadeID);
	void RenderScene(
		const Camera* camera,
		const DirectionalLight* sun,
		const EntityCollection<MeshEntity>& entities,
		uint64_t frameID,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes
