#pragma once

#include "Common.hpp"
#include <InlineMath.hpp>



namespace inl::gxapi {


class IResource {
public:
	virtual ~IResource() = default;

	virtual ResourceDesc GetDesc() const = 0;
	virtual void* Map(unsigned subresourceIndex, const MemoryRange* readRange = nullptr) = 0;
	virtual void Unmap(unsigned subresourceIndex, const MemoryRange* writtenRange = nullptr) = 0;
	virtual void* GetGPUAddress() const = 0;

	virtual unsigned GetNumMipLevels() const = 0;
	virtual unsigned GetNumTexturePlanes() const = 0;
	virtual unsigned GetNumArrayLevels() const = 0;
	virtual unsigned GetNumSubresources() const = 0;
	virtual unsigned GetSubresourceIndex(unsigned mipLevel, unsigned arrayIdx, unsigned planeIdx) const = 0;
	virtual Vec3u64 GetSize(unsigned mipLevel = 0) const = 0;

	// Debug
	virtual void SetName(const char* name) = 0;
};


} // namespace inl::gxapi
