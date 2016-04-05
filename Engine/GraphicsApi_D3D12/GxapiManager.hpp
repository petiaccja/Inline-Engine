#pragma once


#include "../GraphicsApi_LL/IGxapiManager.hpp"


namespace inl {
namespace gxapi_dx12 {


class GxapiManager : public gxapi::IGxapiManager {
public:
	std::vector<gxapi::AdapterInfo> EnumerateAdapters() override;

	gxapi::ISwapChain* CreateSwapChain(gxapi::SwapChainDesc desc, gxapi::ICommandQueue* flushThisQueue) override;
	gxapi::IGraphicsApi* CreateGraphicsApi(unsigned adapterId) override;

	virtual std::vector<std::string> GetShaderIncludeList(exc::Stream& sourceCode) { return{}; };

	bool CompileShader(const exc::Stream& sourceCode,
					   const std::string& mainFunctionName,
					   gxapi::eShaderType type,
					   gxapi::eShaderCompileFlags flags,
					   const std::unordered_map<std::string, exc::Stream*>& includeFiles,
					   const std::vector<gxapi::ShaderMacroDefinition>& macros,
					   gxapi::ShaderProgramBinary& shaderOut,
					   std::string& errorMsg) override;

	gxapi::ShaderProgramBinary CompileShaderFromFile(const std::string& fileName,
							   const std::string& mainFunctionName,
							   gxapi::eShaderType type,
							   gxapi::eShaderCompileFlags flags,
							   const std::vector<gxapi::ShaderMacroDefinition>& macros) override;
};


} // namespace gxapi_dx12
} // namespace inl