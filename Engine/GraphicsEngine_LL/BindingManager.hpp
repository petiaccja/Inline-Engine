#pragma once

#include "RootTableManager.hpp"
#include "ResourceView.hpp"
#include <stdexcept>


namespace inl::gxeng {


template <gxapi::eCommandListType Type>
class BindingManager : protected RootTableManager<Type> {
public:
	BindingManager();
	BindingManager(gxapi::IGraphicsApi* graphicsApi, CommandListT* commandList);

	using RootTableManager::SetBinder;
	using RootTableManager::SetDescriptorHeap;
	using RootTableManager::CommitDrawCall;

	void Bind(BindParameter parameter, const TextureView1D& shaderResource);
	void Bind(BindParameter parameter, const TextureView2D& shaderResource);
	void Bind(BindParameter parameter, const TextureView3D& shaderResource);
	void Bind(BindParameter parameter, const ConstBufferView& shaderConstant);
	void Bind(BindParameter parameter, const void* shaderConstant, int size, int offset);

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
};


template <gxapi::eCommandListType Type>
BindingManager<Type>::BindingManager()
	: RootTableManager()
{}

template <gxapi::eCommandListType Type>
BindingManager<Type>::BindingManager(gxapi::IGraphicsApi* graphicsApi, CommandListT* commandList)
	: RootTableManager(graphicsApi, commandList)
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
		throw std::invalid_argument("Parameter is not an SRV.");
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
		throw std::invalid_argument("Parameter is not a CBV.");
	}
}


template <gxapi::eCommandListType Type>
void BindingManager<Type>::Bind(BindParameter parameter, const void* shaderConstant, int size, int offset) {
	if (size % 4 != 0) {
		throw std::invalid_argument("Size must be a multiple of 4.");
	}
	assert(m_binder != nullptr);

	int slot;
	int tableIndex;
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex); // may throw out of range

	if (desc.rootParameters[slot].type == gxapi::RootParameterDesc::CONSTANT) {
		assert(desc.rootParameters[slot].As<gxapi::RootParameterDesc::CONSTANT>().numConstants >= unsigned(size + offset) / 4);
		SetRootConstants(m_commandList, slot, offset, size / 4, reinterpret_cast<const uint32_t*>(shaderConstant));
	}
	else {
		throw std::invalid_argument("Parameter is not an inline constant.");
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
		throw std::invalid_argument("Parameter is not an UAV.");
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