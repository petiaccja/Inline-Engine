#pragma once

#include "Common.hpp"

#include <string>
#include <unordered_map>
#include <vector>


namespace inl {
class Stream;
};



namespace inl::gxapi {

class ISwapChain;
class IGraphicsApi;
class ICommandQueue;


class IShaderIncludeProvider {
public:
	virtual std::string LoadInclude(const char* includeName, bool systemInclude) = 0;
};


class IGxapiManager {
public:
	virtual ~IGxapiManager() = default;

	virtual std::vector<AdapterInfo> EnumerateAdapters() = 0;
	virtual void EnableDebugLayer() = 0;

	virtual ISwapChain* CreateSwapChain(SwapChainDesc desc, ICommandQueue* flushThisQueue) = 0;
	virtual IGraphicsApi* CreateGraphicsApi(unsigned adapterId) = 0;


	// macro definitions: identifier=value identifier="v a\" \=lue"
	virtual ShaderProgramBinary CompileShader(const char* source,
											  const char* mainFunction,
											  gxapi::eShaderType type,
											  gxapi::eShaderCompileFlags flags,
											  gxapi::IShaderIncludeProvider* includeProvider = nullptr,
											  const char* macroDefinitions = nullptr) = 0;


	virtual ShaderProgramBinary CompileShaderFromFile(const std::string& fileName,
													  const std::string& mainFunctionName,
													  gxapi::eShaderType type,
													  eShaderCompileFlags flags,
													  const std::vector<ShaderMacroDefinition>& macros) = 0;
};


} // namespace inl::gxapi
