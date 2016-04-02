#include "GxapiManager.hpp"
#include "NativeCast.hpp"
#include "SwapChain.hpp"
#include "GraphicsApi.hpp"
#include "ExceptionExpansions.hpp"

#include "../GraphicsApi_LL/Exception.hpp"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include "DisableWin32Macros.h"

#include <string>
#include <cassert>


using Microsoft::WRL::ComPtr;

using namespace inl::gxapi;


namespace inl {
namespace gxapi_dx12 {



class D3DStreamInclude : public ID3DInclude {
public:
	D3DStreamInclude(const std::unordered_map<std::string, exc::Stream*>& streams) : streams(streams) {}

	HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType,
						   LPCSTR pFileName,
						   LPCVOID pParentData,
						   LPCVOID *ppData,
						   UINT *pBytes) override
	{
		try {
			auto it = streams.find(pFileName);
			if (it == streams.end()) {
				return E_FAIL;
			}

			exc::Stream& stream = *it->second;


		}
		catch (std::exception ex) {

		}

	}

	HRESULT __stdcall Close(LPCVOID pData) override {

	}
private:
	const std::unordered_map<std::string, exc::Stream*>& streams;
};



std::vector<AdapterInfo> GxapiManager::EnumerateAdapters() {
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


ISwapChain* GxapiManager::CreateSwapChain(SwapChainDesc desc, ICommandQueue* flushThisQueue) {
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


IGraphicsApi* GxapiManager::CreateGraphicsApi(unsigned adapterId) {
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



bool GxapiManager::CompileShader(const exc::Stream& sourceCode,
								 const std::string& mainFunctionName,
								 gxapi::eShaderType type,
								 eShaderCompileFlags flags,
								 const std::unordered_map<std::string, exc::Stream*>& includeFiles,
								 const std::vector<ShaderMacroDefinition>& macros,
								 ShaderProgramBinary& shaderOut,
								 std::string& errorMsg)
{
	//ID3DBlob *code = nullptr;
	//ID3DBlob *error = nullptr;

	//std::unique_ptr<void> sourceCodeData;
	//size_t sourceCodeSize;
	//std::vector<D3D_SHADER_MACRO> d3dDefines(macros.size());



	//D3DCompile(
	//	sourceCodeData.get(),
	//	sourceCodeSize,
	//	nullptr,
	//	d3dDefines.data(),
	//	d3dIncludes,
	//	mainFuncName,
	//	d3dTarget,
	//	native_cast(flags),
	//	0
	//	& code,
	//	&error
	//	)

	errorMsg = "Method not implemented yet.";
	return false;
}



bool GxapiManager::CompileShaderFromFile(const std::string& fileName,
										 const std::string& mainFunctionName,
										 gxapi::eShaderType type,
										 gxapi::eShaderCompileFlags flags,
										 const std::vector<gxapi::ShaderMacroDefinition>& macros,
										 gxapi::ShaderProgramBinary& shaderOut,
										 std::string& errorMsg)
{
	auto GetTarget = [](eShaderType type) -> const char* {
		switch (type)
		{
			case inl::gxapi::eShaderType::VERTEX:
				return "vs_5_0";
			case inl::gxapi::eShaderType::PIXEL:
				return "ps_5_0";
			case inl::gxapi::eShaderType::DOMAIN:
				return "ds_5_0";
			case inl::gxapi::eShaderType::HULL:
				return "hs_5_0";
			case inl::gxapi::eShaderType::GEOMETRY:
				return "gs_5_0";
			case inl::gxapi::eShaderType::COMPUTE:
				return "cs_5_0";
			default:
				return "invalid";
		}
	};

	// variables
	ID3DBlob *code = nullptr;
	ID3DBlob *error = nullptr;

	std::vector<D3D_SHADER_MACRO> d3dDefines(macros.size()); // native d3d macros
	std::unique_ptr<wchar_t> wFileName(new wchar_t(fileName.size() + 1)); // file name widechar

	// translate defines
	auto defBegin = d3dDefines.begin();
	for (auto& v : macros) {
		defBegin->Name = v.name.c_str();
		defBegin->Definition = v.value.c_str();
		++defBegin;
	}

	// translate file name
	mbstowcs(wFileName.get(), fileName.c_str(), fileName.size());

	// compile code w/ d3d
	HRESULT hr = D3DCompileFromFile(wFileName.get(),
									d3dDefines.data(),
									nullptr,
									mainFunctionName.c_str(),
									GetTarget(type),
									native_cast(flags),
									0,
									&code,
									&error);
	

	if (hr == S_OK) {
		assert(code != nullptr);
		shaderOut.data.resize(code->GetBufferSize());
		memcpy(shaderOut.data.data(), code->GetBufferPointer(), shaderOut.data.size());
	}
	else {
		if (error) {
			errorMsg.assign((char*)error->GetBufferPointer(), error->GetBufferSize());
		}
		if (code) {
			code->Release();
		}
		ThrowIfFailed(hr);
	}

	return true;
}


} // namespace gxapi_dx12
} // namespace inl