#pragma once

#include "Common.hpp"

#include <vector>
#include <string>
#include <unordered_map>


namespace exc {
class Stream;
};



namespace inl {
namespace gxapi {

class ISwapChain;
class IGraphicsApi;
class ICommandQueue;


class IGxapiManager {
public:
	virtual ~IGxapiManager() = default;

	virtual std::vector<AdapterInfo> EnumerateAdapters() = 0;

	virtual ISwapChain* CreateSwapChain(SwapChainDesc desc, ICommandQueue* flushThisQueue) = 0;
	virtual IGraphicsApi* CreateGraphicsApi(unsigned adapterId) = 0;

	virtual std::vector<std::string> GetShaderIncludeList(exc::Stream& sourceCode) = 0;
	virtual bool CompileShader(const exc::Stream& sourceCode,
							   const std::string& mainFunctionName,
							   gxapi::eShaderType type,
							   eShaderCompileFlags flags,
							   const std::unordered_map<std::string, exc::Stream*>& includeFiles,
							   const std::vector<ShaderMacroDefinition>& macros,
							   ShaderProgramBinary& shaderOut,
							   std::string& errorMsg) = 0;
	virtual ShaderProgramBinary CompileShaderFromFile(const std::string& fileName,
									   const std::string& mainFunctionName,
									   gxapi::eShaderType type,
									   eShaderCompileFlags flags,
									   const std::vector<ShaderMacroDefinition>& macros) = 0;
};


} // namespace gxapi
} // namespace inl
