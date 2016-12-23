#include "Node_LightsDeferred.hpp"

namespace inl::gxeng::nodes {


LightsDeferred::LightsDeferred(gxapi::IGraphicsApi * graphicsApi) :
	m_binder(graphicsApi, {})
{

}

void LightsDeferred::RenderScene(
	RenderTargetView & rtv,
	Texture2DSRV & depthStencil,
	Texture2DSRV & albedoRoughness,
	Texture2DSRV & normal,
	const Camera * camera,
	GraphicsCommandList & commandList
) {
	//TODO
}


} // namespace inl::gxeng::nodes
