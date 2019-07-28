#pragma once

#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <BaseLibrary/Platform/Window.hpp>

#include <memory>



class ModuleCollection {
public:
	ModuleCollection(inl::WindowHandle windowHandle);

	inl::gxeng::IGraphicsEngine& GetGraphicsEngine() const;

private:
	std::unique_ptr<inl::gxapi::IGxapiManager> m_gxapiManager;
	std::unique_ptr<inl::gxapi::IGraphicsApi> m_graphicsApi;
	std::unique_ptr<inl::gxeng::IGraphicsEngine> m_graphicsEngine;
	inl::gxapi::AdapterInfo info;
};
