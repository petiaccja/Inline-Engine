#pragma once

#include <BaseLibrary/Platform/Window.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>

#include <memory>



class EngineCollection {
public:
	EngineCollection(inl::WindowHandle windowHandle);

	inl::gxeng::IGraphicsEngine& GetGraphicsEngine() const;
	const inl::gxapi::AdapterInfo& GetGraphicsAdapter() const;

private:
	std::unique_ptr<inl::gxapi::IGxapiManager> m_gxapiManager;
	std::unique_ptr<inl::gxapi::IGraphicsApi> m_graphicsApi;
	std::unique_ptr<inl::gxeng::IGraphicsEngine> m_graphicsEngine;
	inl::gxapi::AdapterInfo m_adapterInfo;
};
