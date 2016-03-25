#include "CantFindAName.hpp"
#include "NativeCast.hpp"


#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "DisableWin32Macros.h"

#include <cstring>


using Microsoft::WRL::ComPtr;

using namespace inl::gxapi;


namespace inl {
namespace gxapi_dx12 {


std::vector<AdapterInfo> CantFindAName::EnumerateAdapters() {
	// create a dxgi factory
	ComPtr<IDXGIFactory4> factory;
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory)))) {
		// it is not supposed to fail unless you call it from a dll main
		// throw some shit anyways
	}


	std::vector<AdapterInfo> adapterInfos;
	ComPtr<IDXGIAdapter1> adapter;


	// enumerate adapters one by one
	unsigned index = 0;
	while (factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		AdapterInfo info;
		adapter->GetDesc1(&desc); // you better not fail

		// fill our info with dxgi info
		char mbs[256];
		wcstombs(mbs, desc.Description, 256);
		info.name = mbs;
		info.adapterId = index;
		info.vendorId = desc.VendorId;
		info.deviceId = desc.DeviceId;
		info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
		info.dedicatedSystemMemory = desc.DedicatedSystemMemory;
		info.sharedSystemMemory = desc.SharedSystemMemory;
		info.isSoftwareAdapter = (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0;
		adapterInfos.push_back(info);

		// next adapter, bitches
		++index;
	}

	return adapterInfos;
}


ISwapChain* CantFindAName::CreateSwapChain(SwapChainDesc desc, ICommandQueue* flushThisQueue) {
	// cast stuff to native counterparts
	DXGI_SWAP_CHAIN_DESC nativeDesc;
	ComPtr<ID3D12CommandQueue> nativeQueue = static_cast<CommandQueue*>(flushThisQueue)->m_native;


	// create a dxgi factory
	ComPtr<IDXGIFactory4> factory;
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory)))) {
		// it is not supposed to fail unless you call it from a dll main
		// throw some shit anyways
	}


	// create the swap chain
	ComPtr<IDXGISwapChain> swapChain;
	if (FAILED(factory->CreateSwapChain(nativeQueue.Get(), &nativeDesc, &swapChain))) {
		// throw error
	}

	
}


IGraphicsApi* CantFindAName::CreateGraphicsApi(unsigned adapterId) {

}



} // namespace gxapi_dx12
} // namespace inl