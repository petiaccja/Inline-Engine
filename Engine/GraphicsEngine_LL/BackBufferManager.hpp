#pragma once

#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/ISwapChain.hpp>
#include "MemoryObject.hpp"


namespace inl {
namespace gxeng {


class RenderTargetView2D;


class BackBufferManager
{
public:
	BackBufferManager(gxapi::IGraphicsApi* graphicsApi, gxapi::ISwapChain* swapChain);

	Texture2D& GetBackBuffer(unsigned index);

protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	gxapi::ISwapChain* m_swapChain;

	std::vector<Texture2D> m_backBuffers;
};


} // namespace gxeng
} // namespace inl
