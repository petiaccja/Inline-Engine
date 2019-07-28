#pragma once

#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <BaseLibrary/Platform/Window.hpp>

#include <memory>



class ModuleCollection {
public:
	ModuleCollection(inl::WindowHandle windowHandle, );

private:
	std::unique_ptr<inl::gxapi::IGxapiManager> gxapiManager;
	std::unique_ptr<inl::gxapi::IGraphicsApi> graphicsApi;
	std::unique_ptr<inl::gxeng::IGraphicsEngine> graphicsEngine;
	gxapi::AdapterInfo info;
};
