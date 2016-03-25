#pragma once


#include "../GraphicsApi_LL/ICantFindAName.hpp"


namespace inl {
namespace gxapi_dx12 {


class CantFindAName : public gxapi::ICantFindAName {
public:
	std::vector<gxapi::AdapterInfo> EnumerateAdapters() override;

	gxapi::ISwapChain* CreateSwapChain(gxapi::SwapChainDesc desc, gxapi::ICommandQueue* flushThisQueue) override;
	gxapi::IGraphicsApi* CreateGraphicsApi(unsigned adapterId) override;
};


} // namespace gxapi_dx12
} // namespace inl