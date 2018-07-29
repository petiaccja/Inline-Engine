#pragma once

#include "RootTableManager.hpp"
#include "ResourceView.hpp"
#include "MemoryManager.hpp"
#include "VolatileViewHeap.hpp"

#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <BaseLibrary/Exception/Exception.hpp>

#include <stdexcept>


namespace inl::gxeng {


template <gxapi::eCommandListType Type>
class BindingManager : protected RootTableManager<Type> {
	using RootTableManager<Type>::CommandListT;
	using RootTableManager<Type>::m_graphicsApi;
	using RootTableManager<Type>::m_commandList;
	using RootTableManager<Type>::m_binder;
	using RootTableManager<Type>::m_heap;
	using RootTableManager<Type>::UpdateBinding;
public:
	BindingManager();
	BindingManager(gxapi::IGraphicsApi* graphicsApi, CommandListT* commandList, MemoryManager* memoryManager, VolatileViewHeap* volatileCbvHeap);

	using RootTableManager<Type>::SetBinder;
	using RootTableManager<Type>::SetDescriptorHeap;
	using RootTableManager<Type>::CommitDrawCall;

	using RootTableManager<Type>::GetDescriptorCounter;

	void Bind(BindParameter parameter, const TextureView1D& shaderResource);
	void Bind(BindParameter parameter, const TextureView2D& shaderResource);
	void Bind(BindParameter parameter, const TextureView3D& shaderResource);
	void Bind(BindParameter parameter, const TextureViewCube& shaderResource);
	void Bind(BindParameter parameter, const ConstBufferView& shaderConstant);

	//! Offset was removed because:
	//! When implicitly creating a CBV to accomodate data, previously set bytes cannot be retrieved, thus bytes before offset cannot be defined.
	void Bind(BindParameter parameter, const void* shaderConstant, int size/*, int offset*/);

	void Bind(BindParameter parameter, const RWTextureView1D& rwResource);
	void Bind(BindParameter parameter, const RWTextureView2D& rwResource);
	void Bind(BindParameter parameter, const RWTextureView3D& rwResource);
	void Bind(BindParameter parameter, const RWBufferView& rwResource);
protected:
	void SetRootConstants(gxapi::IGraphicsCommandList* list, unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value);
	void SetRootConstants(gxapi::IComputeCommandList* list, unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value);
	void SetRootConstantBuffer(gxapi::IGraphicsCommandList* list, unsigned parameterIndex, void* gpuVirtualAddress);
	void SetRootConstantBuffer(gxapi::IComputeCommandList* list, unsigned parameterIndex, void* gpuVirtualAddress);

private:
	void BindTexture(BindParameter parameter, gxapi::DescriptorHandle handle);
	void BindUav(BindParameter parameter, gxapi::DescriptorHandle handle);
private:
	MemoryManager* m_memoryManager;
	VolatileViewHeap* m_volatileCbvHeap;
};


template <gxapi::eCommandListType Type>
BindingManager<Type>::BindingManager()
	: RootTableManager<Type>()
{}

template <gxapi::eCommandListType Type>
BindingManager<Type>::BindingManager(gxapi::IGraphicsApi* graphicsApi, CommandListT* commandList, MemoryManager* memoryManager, VolatileViewHeap* volatileCbvHeap)
	: RootTableManager<Type>(graphicsApi, commandList), m_memoryManager(memoryManager), m_volatileCbvHeap(volatileCbvHeap)
{}


template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const TextureView1D& shaderResource) {
	return BindTexture(parameter, shaderResource.GetHandle());
}


template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const TextureView2D& shaderResource) {
	return BindTexture(parameter, shaderResource.GetHandle());
}


template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const TextureView3D& shaderResource) {
	return BindTexture(parameter, shaderResource.GetHandle());
}

template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const TextureViewCube& shaderResource) {
	return BindTexture(parameter, shaderResource.GetHandle());
}


template <gxapi::eCommandListType Type>
void BindingManager<Type>::BindTexture(BindParameter parameter, gxapi::DescriptorHandle handle) {
	assert(m_binder != nullptr);

	int slot, tableIndex;
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex);
	const auto& rootParam = desc.rootParameters[slot];

	if (rootParam.type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
		UpdateBinding(handle, slot, tableIndex);
	}
	else {
		throw InvalidArgumentException("Parameter is not an SRV.");
	}
}


template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const ConstBufferView& shaderConstant) {
	assert(m_binder != nullptr);

	int slot, tableIndex;
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex);
	const auto& rootParam = desc.rootParameters[slot];

	if (rootParam.type == gxapi::RootParameterDesc::CBV) {
		SetRootConstantBuffer(m_commandList, slot, shaderConstant.GetResource().GetVirtualAddress());
	}
	else if (rootParam.type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
		UpdateBinding(shaderConstant.GetHandle(), slot, tableIndex);
	}
	else {
		throw InvalidArgumentException("Parameter is not a CBV.");
	}
}


template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const void* shaderConstant, int size /*, int offset*/) {
	if (size % 4 != 0) {
		throw InvalidArgumentException("Size must be a multiple of 4.");
	}
	assert(m_binder != nullptr);

	int slot;
	int tableIndex;
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex); // may throw out of range

	if (desc.rootParameters[slot].type == gxapi::RootParameterDesc::CONSTANT) {
		assert(desc.rootParameters[slot].As<gxapi::RootParameterDesc::CONSTANT>().numConstants >= unsigned(size /*+ offset*/) / 4);
		SetRootConstants(m_commandList, slot, /*offset*/0, size / 4, reinterpret_cast<const uint32_t*>(shaderConstant));
	}
	else if (desc.rootParameters[slot].type == gxapi::RootParameterDesc::CBV) {
		// we have to create a volatile CB right here, to accomodate immediate arguments which don't fit in root signature
		VolatileConstBuffer cbuffer = m_memoryManager->CreateVolatileConstBuffer(shaderConstant, size);
		SetRootConstantBuffer(m_commandList, slot, cbuffer.GetVirtualAddress());
	}
	else if (desc.rootParameters[slot].type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
		// we have to create a CBV, and add it to the descriptor table
		VolatileConstBuffer cbuffer = m_memoryManager->CreateVolatileConstBuffer(shaderConstant, size);
		gxapi::DescriptorHandle cbv = m_volatileCbvHeap->Allocate();
		gxapi::ConstantBufferViewDesc desc;
		desc.gpuVirtualAddress = cbuffer.GetVirtualAddress();
		desc.sizeInBytes = size;
		m_graphicsApi->CreateConstantBufferView(desc, cbv);
	}
	else {
		throw InvalidArgumentException("Parameter is not an inline constant.");
	}
}



template <gxapi::eCommandListType Type>
void BindingManager<Type>::BindUav(BindParameter parameter, gxapi::DescriptorHandle handle) {
	assert(m_binder != nullptr);

	int slot, tableIndex;
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex);
	const auto& rootParam = desc.rootParameters[slot];

	if (rootParam.type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
		UpdateBinding(handle, slot, tableIndex);
	}
	else {
		throw InvalidArgumentException("Parameter is not an UAV.");
	}
}

template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const RWTextureView1D& rwResource) {
	return BindUav(parameter, rwResource.GetHandle());
}

template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const RWTextureView2D& rwResource) {
	return BindUav(parameter, rwResource.GetHandle());
}

template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const RWTextureView3D& rwResource) {
	return BindUav(parameter, rwResource.GetHandle());
}

template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const RWBufferView& rwResource) {
	return BindUav(parameter, rwResource.GetHandle());
}


template <gxapi::eCommandListType Type>
void BindingManager<Type>::SetRootConstants(gxapi::IGraphicsCommandList* list, unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value) {
	list->SetGraphicsRootConstants(parameterIndex, destOffset, numValues, value);
}

template <gxapi::eCommandListType Type>
void BindingManager<Type>::SetRootConstants(gxapi::IComputeCommandList* list, unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value) {
	list->SetComputeRootConstants(parameterIndex, destOffset, numValues, value);
}

template <gxapi::eCommandListType Type>
void BindingManager<Type>::SetRootConstantBuffer(gxapi::IGraphicsCommandList* list, unsigned parameterIndex, void* gpuVirtualAddress) {
	list->SetGraphicsRootConstantBuffer(parameterIndex, gpuVirtualAddress);
}

template <gxapi::eCommandListType Type>
void BindingManager<Type>::SetRootConstantBuffer(gxapi::IComputeCommandList* list, unsigned parameterIndex, void* gpuVirtualAddress) {
	list->SetComputeRootConstantBuffer(parameterIndex, gpuVirtualAddress);
}



} // namespace inl::gxeng