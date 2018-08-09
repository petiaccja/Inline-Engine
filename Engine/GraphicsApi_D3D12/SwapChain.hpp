#pragma once

#include "../GraphicsApi_LL/ISwapChain.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include <dxgi1_4.h>
#include <wrl.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {


class SwapChain : public gxapi::ISwapChain {
public:
	SwapChain(Microsoft::WRL::ComPtr<IDXGISwapChain3> native);

	gxapi::IResource* GetBuffer(unsigned index) override;
	gxapi::SwapChainDesc GetDesc() const override;
	bool IsFullScreen() const override;
	unsigned GetCurrentBufferIndex() const override;

	void SetFullScreen(bool isFullScreen) override;
	void Resize(unsigned width, unsigned height, unsigned bufferCount = 0, gxapi::eFormat format = gxapi::eFormat::UNKNOWN) override;

	void Present() override;

private:
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_native;
};

} //namespace gxapi_dx12
} // namespace inl