#pragma once

#include "Common.hpp"

#include <vector>



namespace inl {
namespace gxapi {

class ISwapChain;
class IGraphicsApi;
class ICommandQueue;


class ICantFindAName {
public:
	virtual std::vector<AdapterInfo> EnumerateAdapters() = 0;

	virtual ISwapChain* CreateSwapChain(SwapChainDesc desc, ICommandQueue* flushThisQueue) = 0;
	virtual IGraphicsApi* CreateGraphicsApi(unsigned adapterId) = 0;
};


} // namespace gxapi
} // namespace inl
