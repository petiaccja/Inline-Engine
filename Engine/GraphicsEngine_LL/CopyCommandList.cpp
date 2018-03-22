#include "CopyCommandList.hpp"

namespace inl {
namespace gxeng {


CopyCommandList::CopyCommandList(
	gxapi::IGraphicsApi* gxApi,
	CommandListPool& commandListPool,
	CommandAllocatorPool& commandAllocatorPool,
	ScratchSpacePool& scratchSpacePool
) :
	BasicCommandList(gxApi, commandListPool, commandAllocatorPool, scratchSpacePool, gxapi::eCommandListType::COPY)
{
	m_commandList = dynamic_cast<gxapi::ICopyCommandList*>(GetCommandList());
}


CopyCommandList::CopyCommandList(
	gxapi::IGraphicsApi* gxApi,
	CommandListPool& commandListPool,
	CommandAllocatorPool& commandAllocatorPool,
	ScratchSpacePool& scratchSpacePool,
	gxapi::eCommandListType type
) :
	BasicCommandList(gxApi, commandListPool, commandAllocatorPool, scratchSpacePool, type)
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


void CopyCommandList::SetResourceState(const MemoryObject& resource, gxapi::eResourceState state, unsigned subresource) {
	if (resource.GetHeap() == eResourceHeap::CONSTANT || resource.GetHeap() == eResourceHeap::UPLOAD) {
		throw InvalidArgumentException("You must not set resource state of UPLOAD staging buffers and VOLATILE CONSTANT buffers. They are GENERIC_READ.");
	}

	// Call recursively when ALL subresources are requested.
	if (subresource == gxapi::ALL_SUBRESOURCES) {
		for (unsigned s = 0; s < resource._GetResourcePtr()->GetNumSubresources(); ++s) {
			SetResourceState(resource, state, s);
		}
	}
	// Do a single subresource
	else {
		SubresourceId resId{ resource, subresource };
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
}

// REMOVE THESE IF ONES BELOW WORK
//void CopyCommandList::ExpectResourceState(const MemoryObject& resource, gxapi::eResourceState state, unsigned subresource) {
//	ExpectResourceState(resource, { state }, subresource);
//}
//
//void CopyCommandList::ExpectResourceState(const MemoryObject& resource, const std::initializer_list<gxapi::eResourceState>& anyOfStates, unsigned subresource) {
//	assert(anyOfStates.size() > 0);
//
//	if (resource.GetHeap() == eResourceHeap::CONSTANT || resource.GetHeap() == eResourceHeap::UPLOAD) {
//		return; // they are GENERIC_READ, we don't care about them
//	}
//
//
//	if (subresource == gxapi::ALL_SUBRESOURCES) {
//		for (unsigned s = 0; s < resource._GetResourcePtr()->GetNumSubresources(); ++s) {
//			ExpectResourceState(resource, anyOfStates, s);
//		}
//	}
//	else {
//		SubresourceId resId{ resource, subresource };
//
//		auto iter = m_resourceTransitions.find(resId);
//
//		if (iter == m_resourceTransitions.end()) {
//			if (IsDebuggerPresent()) {
//				DebugBreak();
//			}
//			throw InvalidStateException("You did not set resource state before using this resource!");
//		}
//		else {
//			gxapi::eResourceState currentState = iter->second.lastState;
//			bool ok = false;
//			for (auto it = anyOfStates.begin(); it != anyOfStates.end(); ++it) {
//				ok = ok || ((currentState & *it) == *it);
//			}
//			if (!ok) {
//				if (IsDebuggerPresent()) {
//					DebugBreak();
//				}
//				throw InvalidStateException("You did set resource state, but to the wrong value!");
//			}
//		}
//	}
//}

void CopyCommandList::ExpectResourceState(const MemoryObject& resource, gxapi::eResourceState state, const std::vector<uint32_t>& subresources) {
	ExpectResourceState(resource, { state }, subresources);
}

void CopyCommandList::ExpectResourceState(const MemoryObject& resource, const std::initializer_list<gxapi::eResourceState>& anyOfStates, const std::vector<uint32_t>& subresources) {
	assert(anyOfStates.size() > 0);

	if (resource.GetHeap() == eResourceHeap::CONSTANT || resource.GetHeap() == eResourceHeap::UPLOAD) {
		return; // they are GENERIC_READ, we don't care about them
	}

	struct SubresourceIterator {
		SubresourceIterator(const MemoryObject& resource, const std::vector<uint32_t>& subresources) {
			count = resource.GetNumSubresources();
			sub = &subresources;
			all = false;
			iter = 0;
			for (auto s : subresources) {
				if (s == gxapi::ALL_SUBRESOURCES) {
					all = true;
					return;
				}
			}
		}
		bool all;
		unsigned count;
		unsigned iter;
		const std::vector<uint32_t>* sub;
		bool HasNext() const {
			return all ? (iter < count) : (iter < sub->size());
		}
		void operator++() {
			++iter;
		}
		uint32_t Get() const {
			return all ? (uint32_t)iter : (*sub)[iter];
		}
	};

	SubresourceIterator subiter(resource, subresources);

	while (subiter.HasNext()) {
		uint32_t subres = subiter.Get();

		SubresourceId key{ resource, subres };
		auto iter = m_resourceTransitions.find(key);

		if (iter == m_resourceTransitions.end()) {
			throw InvalidStateException("You must SetSubresourceState before binding the resource view to the pipeline.");
		}
		else {
			gxapi::eResourceState currentState = iter->second.lastState;
			bool ok = false;
			for (auto it = anyOfStates.begin(); it != anyOfStates.end(); ++it) {
				ok = ok || (currentState & *it);
			}
			if (!ok) {
				throw InvalidStateException("Resource being bound to pipeline is in the wrong state. Use SetSubresourceState.");
			}
		}

		++subiter;
	}
}


BasicCommandList::Decomposition CopyCommandList::Decompose() {
	m_commandList = nullptr;
	return BasicCommandList::Decompose();
}


void CopyCommandList::CopyBuffer(const MemoryObject& dst, size_t dstOffset, const MemoryObject& src, size_t srcOffset, size_t numBytes) {
	ExpectResourceState(dst, gxapi::eResourceState::COPY_DEST, { gxapi::ALL_SUBRESOURCES });
	ExpectResourceState(src, gxapi::eResourceState::COPY_SOURCE, { gxapi::ALL_SUBRESOURCES });

	m_commandList->CopyBuffer(dst._GetResourcePtr(), dstOffset, const_cast<gxapi::IResource*>(src._GetResourcePtr()), srcOffset, numBytes);
}


void CopyCommandList::CopyTexture(const Texture2D& dst, const Texture2D& src, SubTexture2D dstPlace, SubTexture2D srcPlace) {
	ExpectResourceState(dst, gxapi::eResourceState::COPY_DEST, { dstPlace.subresource });
	ExpectResourceState(src, gxapi::eResourceState::COPY_SOURCE, { srcPlace.subresource });

	gxapi::TextureCopyDesc dstDesc =
		gxapi::TextureCopyDesc::Texture(dstPlace.subresource);

	gxapi::TextureCopyDesc srcDesc =
		gxapi::TextureCopyDesc::Texture(dstPlace.subresource);

	auto top = std::max(intptr_t(0), srcPlace.corner1.y);
	auto bottom = srcPlace.corner2.y < 0 ? src.GetHeight() : srcPlace.corner2.y;
	auto left = std::max(intptr_t(0), srcPlace.corner1.x);
	auto right = srcPlace.corner2.x < 0 ? src.GetWidth() : srcPlace.corner2.x;

	gxapi::Cube srcRegion((int)top, (int)bottom, (int)left, (int)right, 0, 1);

	auto offsetX = std::max(intptr_t(0), dstPlace.corner1.x);
	auto offsetY = std::max(intptr_t(0), dstPlace.corner1.y);

	m_commandList->CopyTexture(
		dst._GetResourcePtr(),
		dstDesc,
		(int)offsetX, (int)offsetY, 0,
		const_cast<gxapi::IResource*>(src._GetResourcePtr()),
		srcDesc,
		srcRegion
	);
}


void CopyCommandList::CopyTexture(const Texture2D& dst, const Texture2D& src, SubTexture2D dstPlace) {
	ExpectResourceState(dst, gxapi::eResourceState::COPY_DEST, { dstPlace.subresource });
	ExpectResourceState(src, gxapi::eResourceState::COPY_SOURCE, { 0 });

	gxapi::TextureCopyDesc dstDesc =
		gxapi::TextureCopyDesc::Texture(dstPlace.subresource);

	gxapi::TextureCopyDesc srcDesc = gxapi::TextureCopyDesc::Texture(0);

	auto offsetX = std::max(intptr_t(0), dstPlace.corner1.x);
	auto offsetY = std::max(intptr_t(0), dstPlace.corner1.y);

	m_commandList->CopyTexture(
		dst._GetResourcePtr(),
		dstDesc,
		(int)offsetX, (int)offsetY, 0,
		const_cast<gxapi::IResource*>(src._GetResourcePtr()),
		srcDesc
	);
}


void CopyCommandList::CopyTexture(const Texture2D& dst, const LinearBuffer& src, SubTexture2D dstPlace, gxapi::TextureCopyDesc bufferDesc) {
	ExpectResourceState(dst, gxapi::eResourceState::COPY_DEST, { dstPlace.subresource });
	ExpectResourceState(src, gxapi::eResourceState::COPY_SOURCE, { gxapi::ALL_SUBRESOURCES });

	gxapi::TextureCopyDesc dstDesc =
		gxapi::TextureCopyDesc::Texture(dstPlace.subresource);

	m_commandList->CopyTexture(
		dst._GetResourcePtr(),
		dstDesc,
		(int)dstPlace.corner1.x, (int)dstPlace.corner1.y, 0,
		const_cast<gxapi::IResource*>(src._GetResourcePtr()),
		bufferDesc
	);
}



// resource copy
// TODO



} // namespace gxeng
} // namespace inl