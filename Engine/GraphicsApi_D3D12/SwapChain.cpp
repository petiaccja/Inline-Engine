#include "SwapChain.hpp"
#include "Resource.hpp"
#include "NativeCast.hpp"
#include "../GraphicsApi_LL/Exception.hpp"

#include <D3d12.h>
#include <d3dcompiler.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

using namespace inl::gxapi;
using Microsoft::WRL::ComPtr;

namespace inl {
namespace gxapi_dx12 {

SwapChain::SwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain3> native) {
	m_native = native;
	for (unsigned i=0; i<GetDesc().numBuffers; ++i) {
		std::unique_ptr<IResource> buf(GetBuffer(i));
		buf->SetName("BackBuffer");
	}
}


IResource* SwapChain::GetBuffer(unsigned index) {
	ComPtr<ID3D12Resource> resource;
	if (FAILED(m_native->GetBuffer(index, IID_PPV_ARGS(&resource)))) {
		throw OutOfRangeException("You don't have that many swap buffers. Dumbfuck...");
	}
	return new Resource(resource, nullptr);
}


SwapChainDesc SwapChain::GetDesc() const {
	DXGI_SWAP_CHAIN_DESC desc;
	m_native->GetDesc(&desc);
	return native_cast(desc);
}


bool SwapChain::IsFullScreen() const {
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC desc = {};
	m_native->GetFullscreenDesc(&desc);
	return FALSE == desc.Windowed;
}


unsigned SwapChain::GetCurrentBufferIndex() const {
	return m_native->GetCurrentBackBufferIndex();
}


void SwapChain::SetFullScreen(bool isFullScreen) {
	switch (m_native->SetFullscreenState(isFullScreen, nullptr)) {
		case S_OK:
			return;
		case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
			throw RuntimeException("Cannot switch to fullscreen mode now. Try again later.");
		case DXGI_STATUS_MODE_CHANGE_IN_PROGRESS:
			throw InvalidStateException("Already transitioning to fullscreen or windowed.");
		case E_OUTOFMEMORY: 
			throw OutOfMemoryException("Not enough memory for swap chain.");
		default:
			throw Exception();
	}
}


void SwapChain::Resize(unsigned width, unsigned height, unsigned bufferCount, eFormat format) {
	HRESULT err = m_native->ResizeBuffers(bufferCount, width, height, native_cast(format), 0);
	switch (err) {
		case S_OK:
			return;
		case E_OUTOFMEMORY:
			throw OutOfMemoryException("Not enough memory to resize swap chain");
		default:
			throw Exception("Unkown error.");
	}
}


void SwapChain::Present() {
	m_native->Present(0, 0);
}


} // namespace gxapi_dx12
} // namespace inl