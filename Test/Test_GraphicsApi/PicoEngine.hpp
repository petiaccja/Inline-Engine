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
#include <GraphicsApi_LL/IResource.hpp>

namespace exc{
class LogStream;
}


struct VertexBufferView {
	void* gpuAddress;
	unsigned size;
	unsigned stride;
};

struct IndexBufferView {
	void* gpuAddress;
	unsigned size;
	inl::gxapi::eFormat format;
};


class PicoEngine {
public:
	PicoEngine(inl::gxapi::NativeWindowHandle hWnd, int width, int height, exc::LogStream* logStream = nullptr);
	~PicoEngine();

	void Update();
protected:
	static inl::gxapi::ShaderByteCodeDesc ToShaderByteCodeDesc(const inl::gxapi::ShaderProgramBinary& binary);
	//static ShaderByteCodeDesc ToShaderByteCodeDesc(const ShaderProgramBinary& binary);

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
	inl::gxapi::Viewport m_viewport;
	inl::gxapi::Rectangle m_scissorRect;
	int m_width, m_height;

	std::unique_ptr<inl::gxapi::IDescriptorHeap> m_rtvs;
	std::unique_ptr<inl::gxapi::IDescriptorHeap> m_dsv;
	std::vector<std::unique_ptr<inl::gxapi::IResource>> m_depthBuffers;
	int m_currentBackBuffer;
	
	//objects
	std::unique_ptr<inl::gxapi::IResource> m_vertexBuffer;
	std::unique_ptr<inl::gxapi::IResource> m_indexBuffer;
	VertexBufferView vbv;
	IndexBufferView ibv;

	// Logging
	exc::LogStream* m_logStream;
};
