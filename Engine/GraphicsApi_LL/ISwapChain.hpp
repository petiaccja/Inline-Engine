#pragma once

#include "Common.hpp"


namespace inl {
namespace gxapi {

class IResource;


class ISwapChain {
public:
	virtual ~ISwapChain() = default;

	virtual IResource* GetBuffer(unsigned index) = 0;
	virtual SwapChainDesc GetDesc() const = 0;
	virtual bool IsFullScreen() const = 0;
	virtual unsigned GetCurrentBufferIndex() const = 0;

	virtual void SetFullScreen(bool isFullScreen) = 0; 
	virtual void Resize(unsigned width, unsigned height, unsigned bufferCount = 0, eFormat format = eFormat::UNKNOWN) = 0;

	virtual void Present() = 0;
};


} //namespace gxapi
} // namespace inl