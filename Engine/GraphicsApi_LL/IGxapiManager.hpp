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
	virtual std::vector<AdapterInfo> EnumerateAdapters() = 0;

	virtual ISwapChain* CreateSwapChain(SwapChainDesc desc, ICommandQueue* flushThisQueue) = 0;
	virtual IGraphicsApi* CreateGraphicsApi(unsigned adapterId) = 0;

	virtual ShaderProgramBinary CompileShader(exc::Stream& sourceCode,
											  const std::unordered_map<std::string, exc::Stream*>& includeFiles
											  )
};


} // namespace gxapi
} // namespace inl
