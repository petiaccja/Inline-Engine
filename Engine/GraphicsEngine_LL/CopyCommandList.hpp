#pragma once

#include "MemoryObject.hpp"
#include "Binder.hpp"
#include "BasicCommandList.hpp"

#include <GraphicsApi_LL/Common.hpp>
#include <GraphicsApi_LL/ICommandList.hpp>
#include <GraphicsApi_LL/IPipelineState.hpp>

#include <InlineMath.hpp>

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
	SubTexture1D(unsigned subresource = 0,
				 intptr_t firstPixel = -1,
				 intptr_t lastPixel = -1)
		: subresource(subresource),
		firstPixel(firstPixel),
		lastPixel(lastPixel) {}

	unsigned subresource;
	intptr_t firstPixel, lastPixel;
};

struct SubTexture2D {
	SubTexture2D(unsigned subresource = 0,
				Vector<intptr_t, 2> corner1 = { -1, -1 },
				Vector<intptr_t, 2> corner2 = { -1, -1 })
		: subresource(subresource),
		corner1(corner1),
		corner2(corner2) {}

	unsigned subresource;
	Vector<intptr_t, 2> corner1;
	Vector<intptr_t, 2> corner2;
};

struct SubTexture3D {
	SubTexture3D(unsigned subresource = 0,
				 Vector<intptr_t, 3> corner1 = { -1, -1, -1 },
				 Vector<intptr_t, 3> corner2 = { -1, -1, -1 })
		: subresource(subresource),
		corner1(corner1),
		corner2(corner2) {}

	unsigned subresource;
	Vector<intptr_t, 3> corner1;
	Vector<intptr_t, 3> corner2;
};



class CopyCommandList : public BasicCommandList {
public:
	CopyCommandList(
		gxapi::IGraphicsApi* gxApi,
		CommandListPool& commandListPool,
		CommandAllocatorPool& commandAllocatorPool,
		ScratchSpacePool& scratchSpacePool);
	CopyCommandList(const CopyCommandList& rhs) = delete;
	CopyCommandList(CopyCommandList&& rhs);
	CopyCommandList& operator=(const CopyCommandList& rhs) = delete;
	CopyCommandList& operator=(CopyCommandList&& rhs);
protected:
	CopyCommandList(
		gxapi::IGraphicsApi* gxApi,
		CommandListPool& commandListPool,
		CommandAllocatorPool& commandAllocatorPool,
		ScratchSpacePool& scratchSpacePool,
		gxapi::eCommandListType type);

public:
	// Resource copy
	void CopyBuffer(const MemoryObject& dst, size_t dstOffset, const MemoryObject& src, size_t srcOffset, size_t numBytes);

	void CopyResource(const MemoryObject* dst, MemoryObject* src) = delete; // TODO: implement

	void CopyTexture(const Texture1D* dst,
					 const Texture1D* src,
					 SubTexture1D dstPlace = {},
					 SubTexture1D srcPlace = {}) = delete; // TODO: implement
	void CopyTexture(const Texture2D& dst,
					 const Texture2D& src,
					 SubTexture2D dstPlace,
					 SubTexture2D srcPlace);
	void CopyTexture(const Texture2D& dst,
	                 const Texture2D& src,
	                 SubTexture2D dstPlace = {});
	void CopyTexture(const Texture2D& dst,
					 const LinearBuffer& src,
					 SubTexture2D dstPlace,
					 gxapi::TextureCopyDesc bufferDesc);
	void CopyTexture(const Texture3D* dst,
					 const Texture3D* src,
					 SubTexture3D dstPlace = {},
					 SubTexture3D srcPlace = {}) = delete; // TODO: implement


	// barriers
	void SetResourceState(const MemoryObject& resource, gxapi::eResourceState state, unsigned subresource = gxapi::ALL_SUBRESOURCES);
protected:
	void ExpectResourceState(const MemoryObject& resource, gxapi::eResourceState state, const std::vector<uint32_t>& subresources);
	void ExpectResourceState(const MemoryObject& resource, const std::initializer_list<gxapi::eResourceState>& anyOfStates, const std::vector<uint32_t>& subresources);
	//void ExpectResourceState(const MemoryObject& resource, gxapi::eResourceState state, unsigned subresource = gxapi::ALL_SUBRESOURCES);
	//void ExpectResourceState(const MemoryObject& resource, const std::initializer_list<gxapi::eResourceState>& anyOfStates, unsigned subresource = gxapi::ALL_SUBRESOURCES);
	virtual Decomposition Decompose() override;
private:
	gxapi::ICopyCommandList* m_commandList;
};



} // namespace gxeng
} // namespace inl