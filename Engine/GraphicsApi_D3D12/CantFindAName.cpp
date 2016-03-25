#include "CantFindAName.hpp"
#include "NativeCast.hpp"
#include "SwapChain.hpp"
#include "GraphicsApi.hpp"

#include "../GraphicsApi_LL/Exception.hpp"

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
		throw Exception("Are you calling this from a DllMain...? See, that's the problem.");
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
	ComPtr<ID3D12CommandQueue> nativeQueue = native_cast(static_cast<CommandQueue*>(flushThisQueue));


	// create a dxgi factory
	ComPtr<IDXGIFactory4> factory;
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory)))) {
		// it is not supposed to fail unless you call it from a dll main
		throw Exception("Failed to create DXGI factory... Are you calling this from a DllMain?");
	}


	// create the swap chain
	ComPtr<IDXGISwapChain> swapChain;
	switch (factory->CreateSwapChain(nativeQueue.Get(), &nativeDesc, &swapChain)) {
		case S_OK:
			break;
		case E_OUTOFMEMORY: 
			throw OutOfMemory("Not enough memory for swapchain.");
		case DXGI_STATUS_OCCLUDED:
			throw Exception("Full screen mode not available.");
		default:
			throw Exception("Unknown error.");
	}

	ComPtr<IDXGISwapChain3> swapChain3;
	if (FAILED(swapChain.As(&swapChain3))) {
		throw Exception("Could not make IDXGISwapChain3 from IDXGISwapChain.");
	}

	return new SwapChain(swapChain3);
}


IGraphicsApi* CantFindAName::CreateGraphicsApi(unsigned adapterId) {
	// create a dxgi factory
	ComPtr<IDXGIFactory4> factory;
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory)))) {
		// it is not supposed to fail unless you call it from a dll main
		throw Exception("Are you calling this from a DllMain...? See, that's the problem.");
	}


	// get the specified adapter
	ComPtr<IDXGIAdapter1> adapter;
	if (factory->EnumAdapters1(adapterId, &adapter) == DXGI_ERROR_NOT_FOUND) {
		throw OutOfRange("This adapter does not exist. Dumbfuck...");
	}


	// create device w/ adapter
	ComPtr<ID3D12Device> device;
	switch (D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))) {
		case S_OK:
			break;
		default:
			throw Exception("Failed to create D3D12 device.");
	}

	return new GraphicsApi(device);
}



} // namespace gxapi_dx12
} // namespace inl