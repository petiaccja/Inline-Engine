#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include "MemoryObject.hpp"
#include "CommandAllocatorPool.hpp"
#include "ScratchSpacePool.hpp"

#include <vector>
#include <memory>
#include <unordered_map>



namespace inl {
namespace gxeng {

struct SubresourceId {
	SubresourceId() = default;
	SubresourceId(std::shared_ptr<MemoryObject> resource, unsigned subresource) : resource(std::move(resource)), subresource(subresource) {}

	bool operator==(const SubresourceId& other) const {
		return resource == other.resource && subresource == other.subresource;
	}

	std::shared_ptr<MemoryObject> resource;
	unsigned subresource;
};

struct SubresourceUsageInfo {
	gxapi::eResourceState firstState; /// <summary> Holds the target state of the first transition. </summary>
	gxapi::eResourceState lastState; /// <summary> Holds the target state of the last transition. </summary>
	bool multipleStates; /// <sumamry> True if resource was used in more than one state. </summary>
};

struct ResourceUsage {
	std::shared_ptr<MemoryObject> resource;
	unsigned subresource;
	gxapi::eResourceState firstState;
	gxapi::eResourceState lastState;
	bool multipleStates;
};


} // namespace gxeng
} // namespace inl


namespace std {

using namespace inl;

template<>
struct hash<gxeng::SubresourceId> {
	std::size_t operator()(const gxeng::SubresourceId& instance) const {
		return std::hash<gxeng::MemoryObject*>{}(instance.resource.get()) ^ std::hash<unsigned>{}(instance.subresource);
	}
};

} // namespace std



namespace inl {
namespace gxeng {


class BasicCommandList {
public:

	struct Decomposition {
		CmdAllocPtr commandAllocator;
		std::unique_ptr<gxapi::ICopyCommandList> commandList;
		std::vector<ScratchSpacePtr> scratchSpaces;
		std::vector<ResourceUsage> usedResources;
	};
public:
	BasicCommandList(const BasicCommandList& rhs) = delete; // could be, but big perf hit, better not allow user
	BasicCommandList(BasicCommandList&& rhs);
	BasicCommandList& operator=(const BasicCommandList& rhs) = delete; // could be, but big perf hit, better not allow user
	BasicCommandList& operator=(BasicCommandList&& rhs);
	virtual ~BasicCommandList() = default;

	gxapi::eCommandListType GetType() const { return m_commandList->GetType(); }

	virtual Decomposition Decompose();
protected:
	BasicCommandList(
		gxapi::IGraphicsApi* gxApi,
		CommandAllocatorPool& commandAllocatorPool,
		ScratchSpacePool& scratchSpacePool,
		gxapi::eCommandListType type);

	void UseResource(MemoryObject* resource);
	gxapi::ICommandList* GetCommandList() const { return m_commandList.get(); }

	StackDescHeap* GetCurrentScratchSpace();
	void NewScratchSpace(size_t sizeHint);
protected:
	std::unordered_map<SubresourceId, SubresourceUsageInfo> m_resourceTransitions;

private:
	// Part sources
	ScratchSpacePool* m_scratchSpacePool;
	// Parts
	CmdAllocPtr m_commandAllocator;
	std::unique_ptr<gxapi::ICopyCommandList> m_commandList;
	std::vector<ScratchSpacePtr> m_scratchSpaces;
	StackDescHeap* m_currentScratchSpace;
};



} // namespace gxeng
} // namespace inl