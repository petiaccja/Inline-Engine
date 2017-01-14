#pragma once

#include "GraphicsContext.hpp"

#include <vector>

namespace inl::gxeng::nodes {


// These structs provide a solution to pass rextures as render target and shader resource from the same node.
// This is a temporary solution.


struct RenderTargetPack {
	RenderTargetPack() = default;
	RenderTargetPack(uint64_t width, uint32_t height, gxapi::eFormat format, GraphicsContext& context);
	RenderTargetView rtv;
	Texture2DSRV srv;
};


struct DepthStencilPack {
	DepthStencilPack() = default;
	DepthStencilPack(
		uint64_t width,
		uint32_t height,
		gxapi::eFormat formatDepthStencil,
		gxapi::eFormat formatColor,
		gxapi::eFormat formatTypeless,
		GraphicsContext& context);

	DepthStencilView dsv;
	Texture2DSRV srv;
};


struct DepthStencilArrayPack {
	DepthStencilArrayPack() = default;
	DepthStencilArrayPack(
		uint64_t width,
		uint32_t height,
		uint16_t count,
		gxapi::eFormat formatDepthStencil,
		gxapi::eFormat formatColor,
		gxapi::eFormat formatTypeless,
		GraphicsContext& context);

	std::vector<DepthStencilView> dsvs;
	Texture2DSRV srv;
};

} // namespace inl::gxeng::nodes
