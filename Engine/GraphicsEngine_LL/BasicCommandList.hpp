#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include "MemoryObject.hpp"
#include "CommandAllocatorPool.hpp"
#include "CommandListPool.hpp"
#include "ScratchSpacePool.hpp"
#include "HostDescHeap.hpp"

#include <vector>
#include <memory>
#include <unordered_map>



namespace inl {
namespace gxeng {

struct SubresourceId {
	SubresourceId() = default;
	SubresourceId(const MemoryObject& resource, unsigned subresource) : resource(resource), subresource(subresource) {}

	bool operator==(const SubresourceId& other) const {
		return resource == other.resource && subresource == other.subresource;
	}

	MemoryObject resource;
	unsigned subresource;
};

struct SubresourceUsageInfo {
	gxapi::eResourceState firstState; /// <summary> Holds the target state of the first transition. </summary>
	gxapi::eResourceState lastState; /// <summary> Holds the target state of the last transition. </summary>
	bool multipleStates; /// <sumamry> True if resource was used in more than one state. </summary>
};

struct ResourceUsage {
	MemoryObject resource;
	unsigned subresource;
	gxapi::eResourceState firstState;
	gxapi::eResourceState lastState;
	bool multipleStates;
};


struct CommandListCounters {
	size_t numDrawCalls = 0;
	size_t numKernels = 0;
	size_t numScratchSpaceDescriptors = 0;
};



} // namespace gxeng
} // namespace inl


namespace std {

using namespace inl;

template<>
struct hash<gxeng::SubresourceId> {
	std::size_t operator()(const gxeng::SubresourceId& instance) const {
		return std::hash<const void*>{}(instance.resource._GetResourcePtr()) ^ std::hash<unsigned>{}(instance.subresource);
	}
};

} // namespace std



namespace inl {
namespace gxeng {


class BasicCommandList {
public:

	struct Decomposition {
		CmdAllocPtr commandAllocator;
		CmdListPtr commandList;
		std::vector<ScratchSpacePtr> scratchSpaces;
		std::vector<ResourceUsage> usedResources;
		std::vector<MemoryObject> additionalResources;
	};
public:
	BasicCommandList(const BasicCommandList& rhs) = delete; // could be, but big perf hit, better not allow user
	BasicCommandList(BasicCommandList&& rhs);
	BasicCommandList& operator=(const BasicCommandList& rhs) = delete; // could be, but big perf hit, better not allow user
	BasicCommandList& operator=(BasicCommandList&& rhs);
	virtual ~BasicCommandList();

	gxapi::eCommandListType GetType() const { return m_commandList->GetType(); }

	virtual Decomposition Decompose();

	void BeginDebuggerEvent(const std::string& name);
	void EndDebuggerEvent();
	
	void SetName(const std::string& name);
	void SetName(const char* name);

	const CommandListCounters& GetPerformanceCounters() const { return m_performanceCounters; }
protected:
	BasicCommandList(
		gxapi::IGraphicsApi* gxApi,
		CommandListPool& commandListPool,
		CommandAllocatorPool& commandAllocatorPool,
		ScratchSpacePool& scratchSpacePool,
		gxapi::eCommandListType type);

	gxapi::ICommandList* GetCommandList() const { return m_commandList.get(); }

	StackDescHeap* GetCurrentScratchSpace();
	virtual void NewScratchSpace(size_t sizeHint);
protected:
	std::unordered_map<SubresourceId, SubresourceUsageInfo> m_resourceTransitions;
	std::vector<MemoryObject> m_additionalResources;
	gxapi::IGraphicsApi* m_graphicsApi;

	CommandListCounters m_performanceCounters;
private:
	// Part sources
	ScratchSpacePool* m_scratchSpacePool;
	// Parts
	CmdAllocPtr m_commandAllocator;
	CmdListPtr m_commandList;
	std::vector<ScratchSpacePtr> m_scratchSpaces;
	StackDescHeap* m_currentScratchSpace;
};



} // namespace gxeng
} // namespace inl