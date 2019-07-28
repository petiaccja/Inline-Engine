#include "ModuleCollection.hpp"
#include "GraphicsEngine_LL/GraphicsEngine.hpp"
#include "GraphicsApi_D3D12/GxapiManager.hpp"


ModuleCollection::ModuleCollection(inl::WindowHandle windowHandle) {
	m_gxapiManager = std::make_unique<inl::gxapi_dx12::GxapiManager>();

	// Get first hardware adapter, if none, get software, if none, quit.
	auto graphicsAdapters = m_gxapiManager->EnumerateAdapters();
	int graphicsAdapterId = -1;
	inl::gxapi::AdapterInfo info;
	for (auto& adapter : graphicsAdapters) {
		graphicsAdapterId = adapter.adapterId;
		info = adapter;
		if (!adapter.isSoftwareAdapter) {
			break;
		}
	}
	if (graphicsAdapterId == -1) {
		throw inl::RuntimeException("Could not find suitable graphics card or software rendering driver for DirectX 12.");
	}

	m_graphicsApi.reset(m_gxapiManager->CreateGraphicsApi(graphicsAdapterId));

	inl::gxeng::GraphicsEngineDesc graphicsEngineDesc{
		m_gxapiManager.get(),
		m_graphicsApi.get(),
		windowHandle,
		false,
		640,
		480,
	};
	m_graphicsEngine = std::make_unique<inl::gxeng::GraphicsEngine>(graphicsEngineDesc);
}


inl::gxeng::IGraphicsEngine& ModuleCollection::GetGraphicsEngine() const {
	return *m_graphicsEngine;
}
