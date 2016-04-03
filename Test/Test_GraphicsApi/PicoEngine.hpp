#pragma once

#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/ICommandQueue.hpp>
#include <GraphicsApi_LL/ICommandAllocator.hpp>
#include <GraphicsApi_LL/IGraphicsCommandList.hpp>
#include <GraphicsApi_LL/ISwapChain.hpp>
#include <GraphicsApi_LL/IPipelineState.hpp>
#include <GraphicsApi_LL/IRootSignature.hpp>
#include <GraphicsApi_LL/IFence.hpp>
#include <GraphicsApi_LL/IDescriptorHeap.hpp>
#include <GraphicsApi_LL/Common.hpp>

namespace exc{
class LogStream;
}


class PicoEngine {
public:
	PicoEngine(inl::gxapi::NativeWindowHandle hWnd, int width, int height, exc::LogStream* logStream = nullptr);
	~PicoEngine();

	void Update();
protected:
	void Log(std::string s);
	template <class... Args>
	void Log(Args&&... args) {
		if (m_logStream) {
			m_logStream->Event(std::forward<Args>(args)...);
		}
	}

private:
	// gxapi
	std::unique_ptr<inl::gxapi::IGxapiManager> m_gxapiManager;
	std::unique_ptr<inl::gxapi::IGraphicsApi> m_graphicsApi;

	// pipeline
	std::unique_ptr<inl::gxapi::ISwapChain> m_swapChain;
	std::unique_ptr<inl::gxapi::ICommandQueue> m_commandQueue;
	std::unique_ptr<inl::gxapi::ICommandAllocator> m_commandAllocator;
	std::unique_ptr<inl::gxapi::IGraphicsCommandList> m_commandList;
	std::unique_ptr<inl::gxapi::IPipelineState> m_defaultPso;
	std::unique_ptr<inl::gxapi::IRootSignature> m_defaultRootSignature;
	std::unique_ptr<inl::gxapi::IFence> m_fence;

	std::unique_ptr<inl::gxapi::IDescriptorHeap> m_rtvs;
	int currentBackBuffer;
	
	// Logging
	exc::LogStream* m_logStream;
};
