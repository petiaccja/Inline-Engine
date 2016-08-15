#pragma once

#include "CopyCommandList.hpp"


namespace inl {
namespace gxeng {



class ComputeCommandList : public CopyCommandList {
public:
	ComputeCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool);
	ComputeCommandList(const ComputeCommandList& rhs) = delete;
	ComputeCommandList(ComputeCommandList&& rhs);
	ComputeCommandList& operator=(const ComputeCommandList& rhs) = delete;
	ComputeCommandList& operator=(ComputeCommandList&& rhs);
protected:
	ComputeCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool, gxapi::eCommandListType type);

public:
	// set compute root signature stuff
	void SetComputeBinder(Binder* binder);

	void BindCompute(BindParameter parameter, Texture1D* shaderResource);
	void BindCompute(BindParameter parameter, Texture2D* shaderResource);
	void BindCompute(BindParameter parameter, Texture3D* shaderResource);
	void BindCompute(BindParameter parameter, DisposableConstBuffer* shaderConstant);
	void BindCompute(BindParameter parameter, const void* shaderConstant);
	//void BindCompute(BindParameter parameter, RWTexture1D* rwResource);
	//void BindCompute(BindParameter parameter, RWTexture2D* rwResource);
	//void BindCompute(BindParameter parameter, RWTexture3D* rwResource);

protected:
	virtual Decomposition Decompose() override;
private:
	gxapi::IComputeCommandList* m_commandList;
	Binder* m_computeBinder;
};



} // namespace gxeng
} // namespace inl