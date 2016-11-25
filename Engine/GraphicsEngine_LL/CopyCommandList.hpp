#pragma once

#include "MemoryObject.hpp"
#include "Binder.hpp"
#include "BasicCommandList.hpp"

#include "../GraphicsApi_LL/Common.hpp"
#include "../GraphicsApi_LL/ICommandList.hpp"

#include <mathfu/vector_2.h>
#include <type_traits>
#include <unordered_map>

namespace inl {
namespace gxapi {
class ICommandAllocator;
class IPipelineState;
}
}


namespace inl {
namespace gxeng {

class CommandAllocatorPool;

struct SubTexture1D {
	SubTexture1D(unsigned mipLevel = 0,
				 unsigned arrayIndex = 0,
				 intptr_t firstPixel = -1,
				 intptr_t lastPixel = -1)
		: mipLevel(mipLevel),
		arrayIndex(arrayIndex),
		firstPixel(firstPixel),
		lastPixel(lastPixel) {}

	unsigned mipLevel;
	unsigned arrayIndex;
	intptr_t firstPixel, lastPixel;
};

struct SubTexture2D {
	SubTexture2D(unsigned mipLevel = 0,
				 unsigned arrayIndex = 0,
				 mathfu::Vector<intptr_t, 2> corner1 = { -1, -1 },
				 mathfu::Vector<intptr_t, 2> corner2 = { -1, -1 })
		: mipLevel(mipLevel),
		arrayIndex(arrayIndex),
		corner1(corner1),
		corner2(corner2) {}

	unsigned mipLevel;
	unsigned arrayIndex;
	mathfu::Vector<intptr_t, 2> corner1;
	mathfu::Vector<intptr_t, 2> corner2;
};

struct SubTexture3D {
	SubTexture3D(unsigned mipLevel = 0,
				 mathfu::Vector<intptr_t, 3> corner1 = { -1, -1, -1 },
				 mathfu::Vector<intptr_t, 3> corner2 = { -1, -1, -1 })
		: mipLevel(mipLevel),
		corner1(corner1),
		corner2(corner2) {}

	unsigned mipLevel;
	mathfu::Vector<intptr_t, 3> corner1;
	mathfu::Vector<intptr_t, 3> corner2;
};



class CopyCommandList : public BasicCommandList {
public:
	CopyCommandList(
		gxapi::IGraphicsApi* gxApi,
		CommandAllocatorPool& commandAllocatorPool,
		ScratchSpacePool& scratchSpacePool);
	CopyCommandList(const CopyCommandList& rhs) = delete;
	CopyCommandList(CopyCommandList&& rhs);
	CopyCommandList& operator=(const CopyCommandList& rhs) = delete;
	CopyCommandList& operator=(CopyCommandList&& rhs);
protected:
	CopyCommandList(
		gxapi::IGraphicsApi* gxApi,
		CommandAllocatorPool& commandAllocatorPool,
		ScratchSpacePool& scratchSpacePool,
		gxapi::eCommandListType type);

public:
	// Resource copy
	void CopyBuffer(MemoryObject& dst, size_t dstOffset, const MemoryObject& src, size_t srcOffset, size_t numBytes);

	void CopyResource(MemoryObject* dst, MemoryObject* src);

	void CopyTexture(Texture1D* dst,
					 Texture1D* src,
					 SubTexture1D dstPlace = {},
					 SubTexture1D srcPlace = {});
	void CopyTexture(Texture2D& dst,
					 const Texture2D& src,
					 SubTexture2D dstPlace,
					 SubTexture2D srcPlace);
	void CopyTexture(Texture2D& dst,
	                 const Texture2D& src,
	                 SubTexture2D dstPlace = {});
	void CopyTexture(Texture2D& dst,
					 const LinearBuffer& src,
					 SubTexture2D dstPlace,
					 gxapi::TextureCopyDesc bufferDesc);
	void CopyTexture(Texture3D* dst,
					 Texture3D* src,
					 SubTexture3D dstPlace = {},
					 SubTexture3D srcPlace = {});


	// barriers
	void ResourceBarrier(unsigned numBarriers, gxapi::ResourceBarrier* barriers);

	template <class... Barriers>
	void ResourceBarrier(Barriers&&... barriers);

	void SetResourceState(MemoryObject& resource, unsigned subresource, gxapi::eResourceState state);
protected:
	virtual Decomposition Decompose() override;
private:
	gxapi::ICopyCommandList* m_commandList;
};


template<class ...Barriers>
void CopyCommandList::ResourceBarrier(Barriers && ...barriers) {
	m_commandList->ResourceBarrier(barriers...);
}


} // namespace gxeng
} // namespace inl