#include "TextureViewPack.hpp"


namespace inl::gxeng::nodes {


RenderTargetPack::RenderTargetPack(uint64_t width, uint32_t height, gxapi::eFormat format, GraphicsContext& context) {
	Texture2D tex = context.CreateRenderTarget2D(width, height, format, true);

	gxapi::RtvTexture2DArray rtvDesc;
	rtvDesc.activeArraySize = 1;
	rtvDesc.firstArrayElement = 0;
	rtvDesc.planeIndex = 0;
	rtvDesc.firstMipLevel = 0;

	rtv = context.CreateRtv(tex, rtvDesc);

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.numMipLevels = -1;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.planeIndex = 0;

	srv = context.CreateSrv(tex, format, srvDesc);
}


DepthStencilPack::DepthStencilPack(
	uint64_t width,
	uint32_t height,
	gxapi::eFormat formatDepthStencil,
	gxapi::eFormat formatColor,
	gxapi::eFormat formatTypeless,
	GraphicsContext& context
) {
	Texture2D tex = context.CreateDepthStencil2D(width, height, formatTypeless, true);

	gxapi::DsvTexture2DArray dsvDesc;
	dsvDesc.activeArraySize = 1;
	dsvDesc.firstArrayElement = 0;
	dsvDesc.firstMipLevel = 0;

	dsv = context.CreateDsv(tex, formatDepthStencil, dsvDesc);

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = 1;
	srvDesc.firstArrayElement = 0;
	srvDesc.numMipLevels = -1;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.planeIndex = 0;

	srv = context.CreateSrv(tex, formatColor, srvDesc);
}


DepthStencilArrayPack::DepthStencilArrayPack(
	uint64_t width,
	uint32_t height,
	uint16_t count,
	gxapi::eFormat formatDepthStencil,
	gxapi::eFormat formatColor,
	gxapi::eFormat formatTypeless,
	GraphicsContext & context
) {
	Texture2D tex = context.CreateDepthStencil2D(width, height, formatTypeless, true, count);

	gxapi::DsvTexture2DArray dsvDesc;
	dsvDesc.activeArraySize = 1;
	dsvDesc.firstMipLevel = 0;

	dsvs.reserve(count);
	for (uint16_t i = 0; i < count; i++) {
		dsvDesc.firstArrayElement = i;
		dsvs.push_back(context.CreateDsv(tex, formatDepthStencil, dsvDesc));
	}

	gxapi::SrvTexture2DArray srvDesc;
	srvDesc.activeArraySize = count;
	srvDesc.firstArrayElement = 0;
	srvDesc.numMipLevels = -1;
	srvDesc.mipLevelClamping = 0;
	srvDesc.mostDetailedMip = 0;
	srvDesc.planeIndex = 0;

	srv = context.CreateSrv(tex, formatColor, srvDesc);
}


} // namespace inl::gxeng::nodes
