#pragma once

#include "Common.hpp"

namespace inl {
namespace gxapi {

class IResource {
public:
	virtual ~IResource() = default;

	virtual ResourceDesc GetDesc() const = 0;
	virtual void* Map(unsigned subresourceIndex, const MemoryRange* readRange = nullptr) = 0;
	virtual void Unmap(unsigned subresourceIndex, const MemoryRange* writtenRange = nullptr) = 0;
	virtual void* GetGPUAddress() const = 0;

	virtual unsigned GetNumMipLevels() = 0;
	virtual unsigned GetNumTexturePlanes() = 0;
	virtual unsigned GetNumArrayLevels() = 0;

	// Debug
	virtual void SetName(const char* name) = 0;
};

} // namespace gxapi
} // namespace inl
