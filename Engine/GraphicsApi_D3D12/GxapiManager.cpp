#include "GxapiManager.hpp"
#include "NativeCast.hpp"
#include "SwapChain.hpp"
#include "GraphicsApi.hpp"
#include "ExceptionExpansions.hpp"

#include "../GraphicsApi_LL/Exception.hpp"

#include <d3d12.h>
#include <d3dcompiler.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

#include <string>
#include <regex>
#include <cassert>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996) // secure no warning does not work -.-
#endif

using Microsoft::WRL::ComPtr;

using namespace inl::gxapi;


namespace inl {
namespace gxapi_dx12 {



class D3DStreamInclude : public ID3DInclude {
public:
	D3DStreamInclude(const std::unordered_map<std::string, Stream*>& streams) : streams(streams) {}

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

			Stream& stream = *it->second;
		}
		catch (std::exception ex) {

		}

	}

	HRESULT __stdcall Close(LPCVOID pData) override {

	}
private:
	const std::unordered_map<std::string, Stream*>& streams;
};


GxapiManager::GxapiManager() {
	// create a dxgi factory
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory)))) {
		// it is not supposed to fail unless you call it from a dll main
		throw InvalidCallException("Are you calling this from a DllMain...? See, that's the problem.");
	}

	// Enable debug layer
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController))))
	{
		m_debugController->EnableDebugLayer();
	}
}


std::vector<AdapterInfo> GxapiManager::EnumerateAdapters() {
	std::vector<AdapterInfo> adapterInfos;
	ComPtr<IDXGIAdapter1> adapter;


	// enumerate adapters one by one
	unsigned index = 0;
	while (m_factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		AdapterInfo info;
		adapter->GetDesc1(&desc); // you better not fail

		// Exclude microsoft basic renderer
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			++index;
			continue;
		}

		// Exclude if no DX12 support
		if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
			++index;
			continue;
		}

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
		info.isSoftwareAdapter = false;
		adapterInfos.push_back(info);

		// next adapter, bitches
		++index;
	}
	m_numHardwareAdapters = index;

	// enumerate WARP
	if (SUCCEEDED(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)))) {
		DXGI_ADAPTER_DESC1 desc;
		AdapterInfo info;
		adapter->GetDesc1(&desc);

		//char mbs[256];
		//wcstombs(mbs, desc.Description, 256);
		info.name = "WARP12";// mbs;
		info.adapterId = index;
		info.vendorId = desc.VendorId;
		info.deviceId = desc.DeviceId;
		info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
		info.dedicatedSystemMemory = desc.DedicatedSystemMemory;
		info.sharedSystemMemory = desc.SharedSystemMemory;
		info.isSoftwareAdapter = true;
		adapterInfos.push_back(info);
	}

	return adapterInfos;
}


ISwapChain* GxapiManager::CreateSwapChain(SwapChainDesc desc, ICommandQueue* flushThisQueue) {
	// cast stuff to native counterparts
	DXGI_SWAP_CHAIN_DESC nativeDesc = native_cast(desc);
	ComPtr<ID3D12CommandQueue> nativeQueue = native_cast(static_cast<CommandQueue*>(flushThisQueue));


	// create the swap chain
	ComPtr<IDXGISwapChain> swapChain;
	switch (m_factory->CreateSwapChain(nativeQueue.Get(), &nativeDesc, &swapChain)) {
		case S_OK:
			break;
		case E_OUTOFMEMORY:
			throw OutOfMemoryException("Not enough memory for swapchain.");
		case DXGI_STATUS_OCCLUDED:
			throw RuntimeException("Full screen mode not available.");
		default:
			throw Exception();
	}

	ComPtr<IDXGISwapChain3> swapChain3;
	if (FAILED(swapChain.As(&swapChain3))) {
		throw RuntimeException("Could not make IDXGISwapChain3 from IDXGISwapChain.");
	}

	return new SwapChain(swapChain3);
}


IGraphicsApi* GxapiManager::CreateGraphicsApi(unsigned adapterId) {
	// get the specified adapter
	ComPtr<IDXGIAdapter1> adapter;
	if (adapterId == m_numHardwareAdapters) {
		if (FAILED(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)))) {
			throw InvalidArgumentException("Could not acquire WARP adapter.");
		}
	}
	else if (DXGI_ERROR_NOT_FOUND == m_factory->EnumAdapters1(adapterId, &adapter)) {
		throw OutOfRangeException("This adapter does not exist. Dumbfuck...");
	}


	// create device w/ adapter
	ComPtr<ID3D12Device> device;
	switch (D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))) {
		case S_OK:
			break;
		default:
			throw RuntimeException("Failed to create D3D12 device.");
	}

	return new GraphicsApi(device);
}


struct Macro {
	std::string name;
	std::string definition;
};


static std::vector<Macro> ParseMacros(const char* macros) {
	if (macros == nullptr) {
		return{};
	}

	enum {
		NAME,
		VALUE,
	} state = NAME;
	bool escape = false;
	bool quote = false;

	char c;
	size_t i = 0;

	Macro m;
	std::vector<Macro> collection;

	while ((c = macros[i]) != '\0') {
		// escaped characters are just inserted, not processed further
		if (escape) {
			if (state == NAME)
				m.name += c;
			else if (state == VALUE)
				m.definition += c;

			escape = false;
			continue;
		}

		// process characters
		if (c == '"') {
			quote = !quote;
		}
		else if (c == '\\') {
			escape = true;
		}
		else if (c == '=' && !quote) {
			if (state == NAME)
				state = VALUE;
			else
				throw InvalidArgumentException("invalid = sign at character " + std::to_string(i));
		}
		else if (isspace(c) && !quote) {
			// finish off current record on space
			if (state == VALUE) {
				state = NAME;
				collection.push_back(m);
				m.name = m.definition = "";
			}
			if (state == NAME && !m.name.empty()) {
				state = NAME;
				collection.push_back(m);
				m.name = m.definition = "";
			}
		}
		else {
			if (state == NAME)
				m.name += c;
			else if (state == VALUE)
				m.definition += c;
		}
		++i;
	}
	if (!m.name.empty()) {
		collection.push_back(m);
	}

	return collection;
}


class D3dIncludeProvider : public ID3DInclude {
public:
	D3dIncludeProvider(IShaderIncludeProvider* userProvider) : userProvider(userProvider) {}
	virtual ~D3dIncludeProvider() {}

	HRESULT Open(D3D_INCLUDE_TYPE IncludeType,
				 LPCSTR pFileName,
				 LPCVOID pParentData,
				 LPCVOID* ppData,
				 UINT* pBytes) override
	{
		bool systemInclude = IncludeType == D3D_INCLUDE_SYSTEM;

		// search in cache
		std::string searchKey = systemInclude ? "<" : "\"";
		searchKey += pFileName;
		auto it = cache.find(searchKey);
		// if in cache, return that
		if (it != cache.end()) {
			*ppData = it->second.c_str();
			*pBytes = (UINT)it->second.size();
			return S_OK;
		}

		// load by user-provider
		std::string includeData;
		try {
			includeData = userProvider->LoadInclude(pFileName, systemInclude);
		}
		catch (...) {
			*ppData = nullptr;
			*pBytes = 0;
			return E_FAIL;
		}

		// store in cache
		auto insit = cache.insert({ searchKey, includeData });

		// return newly inserted data
		it = insit.first;
		*ppData = it->second.c_str();
		*pBytes = (UINT)it->second.size();
		return S_OK;
	}
	HRESULT Close(LPCVOID pData) override {
		return S_OK;
	}
private:
	std::unordered_map<std::string, std::string> cache;
	IShaderIncludeProvider* userProvider;
};


gxapi::ShaderProgramBinary GxapiManager::CompileShader(
	const char* source,
	const char* mainFunction,
	gxapi::eShaderType type,
	gxapi::eShaderCompileFlags flags,
	gxapi::IShaderIncludeProvider* includeProvider,
	const char* macroDefinitions)
{
	std::vector<Macro> parsedMacroDefinitions = ParseMacros(macroDefinitions);

	ComPtr<ID3DBlob> binaryCode;
	ComPtr<ID3DBlob> errorMessage;
	std::vector<D3D_SHADER_MACRO> d3dMacrosDefines;
	D3dIncludeProvider d3dIncludeProvider(includeProvider);

	for (auto& m : parsedMacroDefinitions) {
		D3D_SHADER_MACRO dm;
		dm.Name = m.name.c_str();
		dm.Definition = m.definition.c_str();
		d3dMacrosDefines.push_back(dm);
	}
	d3dMacrosDefines.push_back({ nullptr, nullptr });

	HRESULT hr = D3DCompile(
		source, strlen(source) + 1,
		nullptr,
		d3dMacrosDefines.data(),
		&d3dIncludeProvider,
		mainFunction,
		GetTarget(type),
		native_cast(flags),
		0,
		&binaryCode,
		&errorMessage);

	if (hr == S_OK) {
		size_t size = binaryCode->GetBufferSize();
		ShaderProgramBinary ret;
		ret.data.resize(size);
		memcpy(ret.data.data(), binaryCode->GetBufferPointer(), size);
		return ret;
	}
	else {
		if (errorMessage.Get() != nullptr) {
			size_t errSize = errorMessage->GetBufferSize();
			std::string errorStr;
			errorStr.resize(errSize);
			memcpy(errorStr.data(), errorMessage->GetBufferPointer(), errSize);

			// This is meant to produce readable file names, but it does not really help.
			//errorStr = std::regex_replace(errorStr, std::regex(R"([[:alnum:]@/_\-\:\\]+(\([0-9\-,]+\):))"), "\\1", std::regex_constants::format_sed);

			throw ShaderCompilationError("Shader compilation failed.", errorStr);
		}
		throw ShaderCompilationError("Failed to compile that crap, but did not get error msg.");
	}
}


gxapi::ShaderProgramBinary GxapiManager::CompileShaderFromFile(const std::string& fileName,
															   const std::string& mainFunctionName,
															   gxapi::eShaderType type,
															   gxapi::eShaderCompileFlags flags,
															   const std::vector<gxapi::ShaderMacroDefinition>& macros)
{
	// variables
	ID3DBlob *code = nullptr;
	ID3DBlob *error = nullptr;

	std::vector<D3D_SHADER_MACRO> d3dDefines(macros.size()); // native d3d macros
	const size_t wFileNameSize = fileName.size() + 1;
	std::unique_ptr<wchar_t> wFileName(new wchar_t[wFileNameSize]); // file name widechar

	// translate defines
	auto defBegin = d3dDefines.begin();
	for (auto& v : macros) {
		defBegin->Name = v.name.c_str();
		defBegin->Definition = v.value.c_str();
		++defBegin;
	}
	d3dDefines.push_back({ NULL, NULL });

	// translate file name
	mbstowcs(wFileName.get(), fileName.c_str(), wFileNameSize);

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

	return ConvertShaderOutput(hr, code, error);
}


const char* GxapiManager::GetTarget(gxapi::eShaderType type) {
	switch (type)
	{
		case inl::gxapi::eShaderType::VERTEX:
			return "vs_5_1";
		case inl::gxapi::eShaderType::PIXEL:
			return "ps_5_1";
		case inl::gxapi::eShaderType::DOMAIN:
			return "ds_5_1";
		case inl::gxapi::eShaderType::HULL:
			return "hs_5_1";
		case inl::gxapi::eShaderType::GEOMETRY:
			return "gs_5_1";
		case inl::gxapi::eShaderType::COMPUTE:
			return "cs_5_1";
	}

	return "invalid";
}


gxapi::ShaderProgramBinary GxapiManager::ConvertShaderOutput(HRESULT hr, ID3DBlob* code, ID3DBlob* error) {
	gxapi::ShaderProgramBinary shaderOut;
	std::string errorMsg;

	if (hr == S_OK) {
		assert(code != nullptr);
		shaderOut.data.resize(code->GetBufferSize());
		memcpy(shaderOut.data.data(), code->GetBufferPointer(), shaderOut.data.size());
		code->Release();
		if (error) error->Release();
	}
	else {
		if (error) {
			errorMsg.assign((char*)error->GetBufferPointer(), error->GetBufferSize());
			error->Release();
		}
		if (code) {
			code->Release();
		}
		throw gxapi::ShaderCompilationError("Error while compiling shader.", errorMsg);
	}

	return shaderOut;
}


} // namespace gxapi_dx12
} // namespace inl