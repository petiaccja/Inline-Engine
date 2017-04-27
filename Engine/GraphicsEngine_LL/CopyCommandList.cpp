#include "CopyCommandList.hpp"

namespace inl {
namespace gxeng {


CopyCommandList::CopyCommandList(
	gxapi::IGraphicsApi* gxApi,
	CommandAllocatorPool& commandAllocatorPool,
	ScratchSpacePool& scratchSpacePool
):
	BasicCommandList(gxApi, commandAllocatorPool, scratchSpacePool, gxapi::eCommandListType::COPY)
{
	m_commandList = dynamic_cast<gxapi::ICopyCommandList*>(GetCommandList());
}


CopyCommandList::CopyCommandList(
	gxapi::IGraphicsApi* gxApi,
	CommandAllocatorPool& commandAllocatorPool,
	ScratchSpacePool& scratchSpacePool,
	gxapi::eCommandListType type
):
	BasicCommandList(gxApi, commandAllocatorPool, scratchSpacePool, type)
{
	m_commandList = dynamic_cast<gxapi::ICopyCommandList*>(GetCommandList());
}


CopyCommandList::CopyCommandList(CopyCommandList&& rhs)
	: BasicCommandList(std::move(rhs)),
	m_commandList(rhs.m_commandList)
{
	rhs.m_commandList = nullptr;
}


CopyCommandList& CopyCommandList::operator=(CopyCommandList&& rhs) {
	BasicCommandList::operator=(std::move(rhs));
	m_commandList = rhs.m_commandList;
	rhs.m_commandList = nullptr;

	return *this;
}


void CopyCommandList::SetResourceState(const MemoryObject& resource, unsigned subresource, gxapi::eResourceState state) {
	subresource = gxapi::ALL_SUBRESOURCES; // until there's no validation on subresources
	SubresourceId resId{resource, subresource};
	auto iter = m_resourceTransitions.find(resId);
	bool firstTransition = iter == m_resourceTransitions.end();
	if (firstTransition) {
		SubresourceUsageInfo info;
		info.lastState = state;
		info.firstState = state;
		info.multipleStates = false;
		m_resourceTransitions.insert({ std::move(resId), info });
	}
	else {
		const auto& prevState = iter->second.lastState;

		if (prevState != state) {
			m_commandList->ResourceBarrier(
				gxapi::TransitionBarrier{
					resource._GetResourcePtr(),
					prevState,
					state,
					subresource
				}
			);
			iter->second.lastState = state;
			iter->second.multipleStates = true;
		}
	}
}


void CopyCommandList::ExpectResourceState(const MemoryObject& resource, unsigned subresource, gxapi::eResourceState state) {
	SubresourceId resId{ resource, gxapi::ALL_SUBRESOURCES };
	auto iter = m_resourceTransitions.find(resId);
	if (iter == m_resourceTransitions.end()) {
		if (IsDebuggerPresent()) {
			DebugBreak();
		}
		throw std::logic_error("You did not set resource state before using this resource!");
	}
	else if (!(iter->second.lastState & state)) {
		if (IsDebuggerPresent()) {
			DebugBreak();
		}
		throw std::logic_error("You did set resource state, but to the wrong value!");
	}
}


BasicCommandList::Decomposition CopyCommandList::Decompose() {
	m_commandList = nullptr;
	return BasicCommandList::Decompose();
}




void CopyCommandList::CopyBuffer(MemoryObject& dst, size_t dstOffset, const MemoryObject& src, size_t srcOffset, size_t numBytes) {
	m_commandList->CopyBuffer(dst._GetResourcePtr(), dstOffset, const_cast<gxapi::IResource*>(src._GetResourcePtr()), srcOffset, numBytes);
}


void CopyCommandList::CopyTexture(Texture2D& dst, const Texture2D& src, SubTexture2D dstPlace, SubTexture2D srcPlace) {
	gxapi::TextureCopyDesc dstDesc =
		gxapi::TextureCopyDesc::Texture(dst.GetSubresourceIndex(dstPlace.arrayIndex, dstPlace.mipLevel));

	gxapi::TextureCopyDesc srcDesc =
		gxapi::TextureCopyDesc::Texture(src.GetSubresourceIndex(srcPlace.arrayIndex, srcPlace.mipLevel));

	auto top = std::max(intptr_t(0), srcPlace.corner1.y());
	auto bottom = srcPlace.corner2.y() < 0 ? src.GetHeight() : srcPlace.corner2.y();
	auto left = std::max(intptr_t(0), srcPlace.corner1.x());
	auto right = srcPlace.corner2.x() < 0 ? src.GetWidth() : srcPlace.corner2.x();

	gxapi::Cube srcRegion((int)top, (int)bottom, (int)left, (int)right, 0, 1);

	auto offsetX = std::max(intptr_t(0), dstPlace.corner1.x());
	auto offsetY = std::max(intptr_t(0), dstPlace.corner1.y());

	m_commandList->CopyTexture(
		dst._GetResourcePtr(),
		dstDesc,
		(int)offsetX, (int)offsetY, 0,
		const_cast<gxapi::IResource*>(src._GetResourcePtr()),
		srcDesc,
		srcRegion
	);
}


void CopyCommandList::CopyTexture(Texture2D& dst, const Texture2D& src, SubTexture2D dstPlace) {
	gxapi::TextureCopyDesc dstDesc =
		gxapi::TextureCopyDesc::Texture(dst.GetSubresourceIndex(dstPlace.arrayIndex, dstPlace.mipLevel));

	gxapi::TextureCopyDesc srcDesc = gxapi::TextureCopyDesc::Texture(0);

	auto offsetX = std::max(intptr_t(0), dstPlace.corner1.x());
	auto offsetY = std::max(intptr_t(0), dstPlace.corner1.y());

	m_commandList->CopyTexture(
		dst._GetResourcePtr(),
		dstDesc,
		(int)offsetX, (int)offsetY, 0,
		const_cast<gxapi::IResource*>(src._GetResourcePtr()),
		srcDesc
	);
}


void CopyCommandList::CopyTexture(Texture2D& dst, const LinearBuffer& src, SubTexture2D dstPlace, gxapi::TextureCopyDesc bufferDesc) {
	gxapi::TextureCopyDesc dstDesc =
		gxapi::TextureCopyDesc::Texture(dst.GetSubresourceIndex(dstPlace.arrayIndex, dstPlace.mipLevel));

	m_commandList->CopyTexture(
		dst._GetResourcePtr(),
		dstDesc,
		(int)dstPlace.corner1.x(), (int)dstPlace.corner1.y(), 0,
		const_cast<gxapi::IResource*>(src._GetResourcePtr()),
		bufferDesc
	);
}



// resource copy
// TODO



} // namespace gxeng
} // namespace inl