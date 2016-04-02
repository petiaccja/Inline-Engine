#pragma once

#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/ICommandQueue.hpp>
#include <GraphicsApi_LL/ICommandAllocator.hpp>
#include <GraphicsApi_LL/ICommandList.hpp>

class PicoEngine {
public:
	PicoEngine();


private:
	// gxapi
	std::unique_ptr<inl::gxapi::IGxapiManager> gxapiManager;
	std::unique_ptr<inl::gxapi::IGraphicsApi> graphicsApi;

	// pipeline
	std::unique_ptr<inl::gxapi::ICommandQueue> commandQueue;
	std::unique_ptr<inl::gxapi::ICommandAllocator> commandAllocator;
	std::unique_ptr<inl::gxapi::ICommandList> commandList;
};
