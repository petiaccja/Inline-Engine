#pragma once

#include "CopyCommandList.hpp"
#include "BindingManager.hpp"


namespace inl {
namespace gxeng {



class ComputeCommandList : public CopyCommandList {
public:
	ComputeCommandList(gxapi::IGraphicsApi* gxApi,
					   CommandListPool& commandListPool,
					   CommandAllocatorPool& commandAllocatorPool,
					   ScratchSpacePool& scratchSpacePool,
					   MemoryManager& memoryManager,
					   VolatileViewHeap& volatileCbvHeap);
	ComputeCommandList(const ComputeCommandList& rhs) = delete;
	ComputeCommandList(ComputeCommandList&& rhs);
	ComputeCommandList& operator=(const ComputeCommandList& rhs) = delete;
	ComputeCommandList& operator=(ComputeCommandList&& rhs);
protected:
	ComputeCommandList(gxapi::IGraphicsApi* gxApi,
					   CommandListPool& commandListPool,
					   CommandAllocatorPool& commandAllocatorPool,
					   ScratchSpacePool& scratchSpacePool,
					   MemoryManager& memoryManager,
					   VolatileViewHeap& volatileCbvHeap,
					   gxapi::eCommandListType type);

public:
	// Draw
	void Dispatch(size_t numThreadGroupsX, size_t numThreadGroupsY, size_t numThreadGroupsZ);

	// Command list state
	void ResetState(gxapi::IPipelineState* newState = nullptr);
	void SetPipelineState(gxapi::IPipelineState* pipelineState);

	// set compute root signature stuff
	void SetComputeBinder(const Binder* binder);

	void BindCompute(BindParameter parameter, const TextureView1D& shaderResource);
	void BindCompute(BindParameter parameter, const TextureView2D& shaderResource);
	void BindCompute(BindParameter parameter, const TextureView3D& shaderResource);
	void BindCompute(BindParameter parameter, const ConstBufferView& shaderConstant);
	void BindCompute(BindParameter parameter, const void* shaderConstant, int size/*, int offset*/);
	void BindCompute(BindParameter parameter, const RWTextureView1D& rwResource);
	void BindCompute(BindParameter parameter, const RWTextureView2D& rwResource);
	void BindCompute(BindParameter parameter, const RWTextureView3D& rwResource);
	void BindCompute(BindParameter parameter, const RWBufferView& rwResource);

	// UAV barriers
	void UAVBarrier(const MemoryObject& memoryObject);
protected:
	virtual Decomposition Decompose() override;
	virtual void NewScratchSpace(size_t hint) override;
private:
	gxapi::IComputeCommandList* m_commandList;

	// scratch space managment
	BindingManager<gxapi::eCommandListType::COMPUTE> m_computeBindingManager;
};



} // namespace gxeng
} // namespace inl