#include "Node_LightsDeferred.hpp"

namespace inl::gxeng::nodes {


LightsDeferred::LightsDeferred(gxapi::IGraphicsApi * graphicsApi) :
	m_binder(graphicsApi, {})
{

}

void LightsDeferred::RenderScene(
	RenderTargetView2D & rtv,
	TextureView2D & depthStencil,
	TextureView2D & albedoRoughness,
	TextureView2D & normal,
	const Camera * camera,
	GraphicsCommandList & commandList
) {
	//TODO
}


} // namespace inl::gxeng::nodes
