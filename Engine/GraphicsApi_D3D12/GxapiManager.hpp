#pragma once

#include "../GraphicsApi_LL/IGxapiManager.hpp"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>


namespace inl::gxapi_dx12 {


class GxapiManager : public gxapi::IGxapiManager {
public:
	GxapiManager();

	std::vector<gxapi::AdapterInfo> EnumerateAdapters() override;

	gxapi::ISwapChain* CreateSwapChain(gxapi::SwapChainDesc desc, gxapi::ICommandQueue* flushThisQueue) override;
	gxapi::IGraphicsApi* CreateGraphicsApi(unsigned adapterId) override;


	gxapi::ShaderProgramBinary CompileShader(const char* source,
											 const char* mainFunction,
											 gxapi::eShaderType type,
											 gxapi::eShaderCompileFlags flags,
											 gxapi::IShaderIncludeProvider* includeProvider = nullptr,
											 const char* macroDefinitions = nullptr) override;

	gxapi::ShaderProgramBinary CompileShaderFromFile(const std::string& fileName,
													 const std::string& mainFunctionName,
													 gxapi::eShaderType type,
													 gxapi::eShaderCompileFlags flags,
													 const std::vector<gxapi::ShaderMacroDefinition>& macros) override;

protected:
	static const char* GetTarget(gxapi::eShaderType type);
	static gxapi::ShaderProgramBinary ConvertShaderOutput(HRESULT hr, ID3DBlob* code, ID3DBlob* error);
private:
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
	Microsoft::WRL::ComPtr<ID3D12Debug> m_debugController;
	unsigned m_numHardwareAdapters = 0;
};


} // namespace gxapi_dx12