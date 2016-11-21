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


void CopyCommandList::SetResourceState(std::shared_ptr<MemoryObject> resource, unsigned subresource, gxapi::eResourceState state) {
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
			ResourceBarrier(
				gxapi::TransitionBarrier{
					resource->_GetResourcePtr(),
					prevState,
					state,
					subresource
				}
			);
		}

		iter->second.lastState = state;
		iter->second.multipleStates = true;
	}
}


BasicCommandList::Decomposition CopyCommandList::Decompose() {
	m_commandList = nullptr;
	return BasicCommandList::Decompose();
}




void CopyCommandList::CopyBuffer(MemoryObject * dst, size_t dstOffset, MemoryObject * src, size_t srcOffset, size_t numBytes) {
	m_commandList->CopyBuffer(dst->_GetResourcePtr(), dstOffset, src->_GetResourcePtr(), srcOffset, numBytes);
}


void CopyCommandList::CopyTexture(Texture2D* dst, Texture2D* src, SubTexture2D dstPlace, SubTexture2D srcPlace) {
	gxapi::TextureCopyDesc dstDesc =
		gxapi::TextureCopyDesc::Texture(dst->GetSubresourceIndex(dstPlace.arrayIndex, dstPlace.mipLevel));

	gxapi::TextureCopyDesc srcDesc =
		gxapi::TextureCopyDesc::Texture(src->GetSubresourceIndex(srcPlace.arrayIndex, srcPlace.mipLevel));

	auto top = std::max(0, srcPlace.corner1.y());
	auto bottom = srcPlace.corner2.y() < 0 ? src->GetHeight() : srcPlace.corner2.y();
	auto left = std::max(0, srcPlace.corner1.x());
	auto right = srcPlace.corner2.x() < 0 ? src->GetWidth() : srcPlace.corner2.x();

	gxapi::Cube srcRegion(top, bottom, left, right, 0, 1);

	auto offsetX = std::max(0, dstPlace.corner1.x());
	auto offsetY = std::max(0, dstPlace.corner1.y());

	m_commandList->CopyTexture(dst->_GetResourcePtr(), dstDesc, offsetX, offsetY, 0, src->_GetResourcePtr(), srcDesc, srcRegion);
}


void CopyCommandList::CopyTexture(Texture2D* dst, Texture2D* src, SubTexture2D dstPlace) {
	gxapi::TextureCopyDesc dstDesc =
		gxapi::TextureCopyDesc::Texture(dst->GetSubresourceIndex(dstPlace.arrayIndex, dstPlace.mipLevel));

	gxapi::TextureCopyDesc srcDesc = gxapi::TextureCopyDesc::Texture(0);

	auto offsetX = std::max(0, dstPlace.corner1.x());
	auto offsetY = std::max(0, dstPlace.corner1.y());

	m_commandList->CopyTexture(dst->_GetResourcePtr(), dstDesc, offsetX, offsetY, 0, src->_GetResourcePtr(), srcDesc);
}


void CopyCommandList::CopyTexture(Texture2D* dst, LinearBuffer* src, SubTexture2D dstPlace, gxapi::TextureCopyDesc bufferDesc) {
	gxapi::TextureCopyDesc dstDesc =
		gxapi::TextureCopyDesc::Texture(dst->GetSubresourceIndex(dstPlace.arrayIndex, dstPlace.mipLevel));

	m_commandList->CopyTexture(dst->_GetResourcePtr(), dstDesc, dstPlace.corner1.x(), dstPlace.corner1.y(), 0, src->_GetResourcePtr(), bufferDesc);
}



// resource copy
// TODO


// barriers
void CopyCommandList::ResourceBarrier(unsigned numBarriers, gxapi::ResourceBarrier* barriers) {
	m_commandList->ResourceBarrier(numBarriers, barriers);
}




} // namespace gxeng
} // namespace inl