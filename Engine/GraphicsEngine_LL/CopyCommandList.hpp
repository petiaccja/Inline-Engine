#pragma once

#include "GpuBuffer.hpp"
#include "Binder.hpp"
#include "BasicCommandList.hpp"
#include "SubresourceID.hpp"

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


struct StateTransitionRegister {
	gxapi::eResourceState lastTargetState; // Holds the target state of the last transition.

	gxapi::eResourceState firstTargetState; // Holds the target state of the first transition. (Relevant if multipleTransition is true)
	bool multipleTransition;
};


struct SubTexture1D {
	SubTexture1D(unsigned mipLevel = 0,
				 unsigned arrayIndex = 0,
				 int firstPixel = -1,
				 int lastPixel = -1)
		: mipLevel(mipLevel),
		arrayIndex(arrayIndex),
		firstPixel(firstPixel),
		lastPixel(lastPixel) {}

	unsigned mipLevel;
	unsigned arrayIndex;
	int firstPixel, lastPixel;
};

struct SubTexture2D {
	SubTexture2D(unsigned mipLevel = 0,
				 unsigned arrayIndex = 0,
				 mathfu::Vector<int, 2> corner1 = { -1, -1 },
				 mathfu::Vector<int, 2> corner2 = { -1, -1 })
		: mipLevel(mipLevel),
		arrayIndex(arrayIndex),
		corner1(corner1),
		corner2(corner2) {}

	unsigned mipLevel;
	unsigned arrayIndex;
	mathfu::Vector<int, 2> corner1;
	mathfu::Vector<int, 2> corner2;
};

struct SubTexture3D {
	SubTexture3D(unsigned mipLevel = 0,
				 mathfu::Vector<int, 3> corner1 = { -1, -1, -1 },
				 mathfu::Vector<int, 3> corner2 = { -1, -1, -1 })
		: mipLevel(mipLevel),
		corner1(corner1),
		corner2(corner2) {}

	unsigned mipLevel;
	mathfu::Vector<int, 3> corner1;
	mathfu::Vector<int, 3> corner2;
};



class CopyCommandList : public BasicCommandList {
public:
	CopyCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool);
	CopyCommandList(const CopyCommandList& rhs) = delete;
	CopyCommandList(CopyCommandList&& rhs);
	CopyCommandList& operator=(const CopyCommandList& rhs) = delete;
	CopyCommandList& operator=(CopyCommandList&& rhs);
protected:
	CopyCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool, gxapi::eCommandListType type);

public:
	// Command list state
	void ResetState(gxapi::IPipelineState* newState = nullptr);


	// Resource copy
	void CopyBuffer(GenericResource* dst, size_t dstOffset, GenericResource* src, size_t srcOffset, size_t numBytes);

	void CopyResource(GenericResource* dst, GenericResource* src);

	void CopyTexture(Texture1D* dst,
					 Texture1D* src,
					 SubTexture1D dstPlace = {},
					 SubTexture1D srcPlace = {});
	void CopyTexture(Texture2D* dst,
					 Texture2D* src,
					 SubTexture2D dstPlace = {},
					 SubTexture2D srcPlace = {});
	void CopyTexture(Texture3D* dst,
					 Texture3D* src,
					 SubTexture3D dstPlace = {},
					 SubTexture3D srcPlace = {});


	// barriers
	void ResourceBarrier(unsigned numBarriers, gxapi::ResourceBarrier* barriers);

	template <class... Barriers>
	void ResourceBarrier(Barriers&&... barriers);

	void RegisterResourceTransition(const SubresourceID& subresource, gxapi::eResourceState targetState);
	std::unordered_map<SubresourceID, StateTransitionRegister>& GetResourceTransitions() { return m_resourceTransitions; }

protected:
	virtual Decomposition Decompose() override;
private:
	gxapi::ICopyCommandList* m_commandList;
	std::unordered_map<SubresourceID, StateTransitionRegister> m_resourceTransitions;
};


template<class ...Barriers>
void CopyCommandList::ResourceBarrier(Barriers && ...barriers) {
	m_commandList->ResourceBarrier(barriers...);
}


} // namespace gxeng
} // namespace inl