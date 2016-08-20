#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include "GpuBuffer.hpp"
#include "CommandAllocatorPool.hpp"
#include "ScratchSpacePool.hpp"

#include <vector>
#include <memory>


namespace inl {
namespace gxeng {



class BasicCommandList {
	using CmdListPtr = std::unique_ptr<gxapi::ICommandList>;
public:

	struct Decomposition {
		CmdAllocPtr commandAllocator;
		CmdListPtr commandList;
		std::vector<GenericResource*> usedResources;
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
	BasicCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool, gxapi::eCommandListType type);
	void UseResource(GenericResource* resource);
	gxapi::ICommandList* GetCommandList() const { return m_commandList.get(); }

private:
	// Resources
	std::vector<GenericResource*> m_usedResources;

	// Part sources
	ScratchSpacePool* m_scratchSpacePool;
	// Parts
	std::vector<ScratchSpacePtr> m_scratchSpaces;
	CmdAllocPtr m_commandAllocator;
	CmdListPtr m_commandList;
	ScratchSpace* m_currentScratchSpace;
};



} // namespace gxeng
} // namespace inl