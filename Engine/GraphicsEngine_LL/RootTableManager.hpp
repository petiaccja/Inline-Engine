#pragma once

#include <vector>
#include <utility>
#include <cassert>
#include <type_traits>
#include <GraphicsApi_LL/ICommandList.hpp>
#include "StackDescHeap.hpp"
#include "Binder.hpp"


namespace inl::gxeng {


struct DescriptorTableState {
	DescriptorTableState() : slot(0), committed(false) {}
	DescriptorTableState(DescriptorArrayRef&& reference, int slot)
		: reference(std::move(reference)), slot(slot), committed(false)
	{}

	DescriptorArrayRef reference; // current place in scratch space
	int slot; // which root signature slot it belongs to
	bool committed; // true if modifying descriptor in sratch space would break previous draw calls
	std::vector<gxapi::DescriptorHandle> bindings; // currently bound descriptor handle, staging heap sources
};



template <gxapi::eCommandListType Type>
class RootTableManager {
protected:
	using CommandListT = typename std::conditional<
		gxapi::eCommandListType::GRAPHICS == Type,
		gxapi::IGraphicsCommandList,
		gxapi::IComputeCommandList>::type;
public:
	RootTableManager();
	RootTableManager(gxapi::IGraphicsApi* graphicsApi, CommandListT* commandList);
	void SetBinder(Binder* binder);
	void SetDescriptorHeap(StackDescHeap* heap);
	void CommitDrawCall();
	void UpdateBinding(gxapi::DescriptorHandle handle, int rootSignatureSlot, int indexInTable);
private:
	/// <summary> Updates a binding which is managed on the scratch space. </summary>
	void UpdateRootTable(gxapi::DescriptorHandle, int rootSignatureSlot, int indexInTable);

	/// <summary> Copies a whole scratch space table to a fresh range in scratch space. </summary>
	void DuplicateRootTable(DescriptorTableState& table);

	/// <summary> Get reference to root table state identified by it's root signature slot. </summary>
	DescriptorTableState&  FindRootTable(int rootSignatureSlot);

	/// <summary> Calculates root table states based on the currently bound Binder. </summary>
	void InitRootTables();

	/// <summary> Marks all root tables committed. Call this after each drawcall. </summary>
	void CommitRootTables();

	/// <summary> Copies ALL scratch space tables to a fresh range. Used after a new scratch space is bound. </summary>
	void RenewRootTables();

	void SetRootDescriptorTable(gxapi::IGraphicsCommandList* list, unsigned parameterIndex, gxapi::DescriptorHandle baseHandle);
	void SetRootDescriptorTable(gxapi::IComputeCommandList* list, unsigned parameterIndex, gxapi::DescriptorHandle baseHandle);
	void SetRootSignature(gxapi::IGraphicsCommandList* list, gxapi::IRootSignature* sig);
	void SetRootSignature(gxapi::IComputeCommandList* list, gxapi::IRootSignature* sig);
protected:
	gxapi::IGraphicsApi* m_graphicsApi;
	CommandListT* m_commandList;
	Binder* m_binder;
	StackDescHeap* m_heap;
private:
	std::vector<DescriptorTableState> m_rootTableStates;
};



template <gxapi::eCommandListType Type>
RootTableManager<Type>::RootTableManager() {
	m_graphicsApi = nullptr;
	m_commandList = nullptr;
}


template <gxapi::eCommandListType Type>
RootTableManager<Type>::RootTableManager(gxapi::IGraphicsApi* graphicsApi, CommandListT* commandList) {
	m_graphicsApi = graphicsApi;
	m_commandList = commandList;
}


template <gxapi::eCommandListType Type>
void RootTableManager<Type>::SetBinder(Binder* binder) {
	m_binder = binder;
	SetRootSignature(m_commandList, m_binder->GetRootSignature());
	InitRootTables();
}


template <gxapi::eCommandListType Type>
void RootTableManager<Type>::SetDescriptorHeap(StackDescHeap* heap) {
	assert(heap != nullptr);
	m_heap = heap;
	RenewRootTables();
}


template <gxapi::eCommandListType Type>
void RootTableManager<Type>::CommitDrawCall() {
	CommitRootTables();
}


template <gxapi::eCommandListType Type>
void RootTableManager<Type>::UpdateBinding(gxapi::DescriptorHandle handle, int rootSignatureSlot, int indexInTable) {
	UpdateRootTable(handle, rootSignatureSlot, indexInTable);
}


template <gxapi::eCommandListType Type>
void RootTableManager<Type>::UpdateRootTable(gxapi::DescriptorHandle handle, int rootSignatureSlot, int indexInTable) {
	DescriptorTableState& table = FindRootTable(rootSignatureSlot);

	// if table is committed, duplicate it so that recent drawcalls won't be broken
	if (table.committed) {
		// update handle in advance so that duplicate will copy it instead and we save time
		table.bindings[indexInTable] = handle;
		DuplicateRootTable(table);

		// update table root parameters
		SetRootDescriptorTable(m_commandList, rootSignatureSlot, table.reference.Get(0));
	}
	// just update the binding
	else {
		table.bindings[indexInTable] = handle;
		m_graphicsApi->CopyDescriptors(handle, table.reference.Get(indexInTable), 1, gxapi::eDescriptorHeapType::CBV_SRV_UAV);
	}
}

template <gxapi::eCommandListType Type>
void RootTableManager<Type>::DuplicateRootTable(DescriptorTableState& table) {
	uint32_t numDescriptors = (uint32_t)table.bindings.size();

	// allocate new space on scratch space
	DescriptorArrayRef space = m_heap->Allocate(numDescriptors);

	// copy old descriptors to new space
	std::vector<gxapi::DescriptorHandle> sourceDescHandles(numDescriptors);
	std::vector<uint32_t> sourceRangeSizes(numDescriptors, 1);
	for (size_t i = 0; i < numDescriptors; ++i) {
		sourceDescHandles[i] = table.bindings[i];
	}

	gxapi::DescriptorHandle destDescHandle = space.Get(0);

	m_graphicsApi->CopyDescriptors(
		sourceDescHandles.size(), sourceDescHandles.data(), sourceRangeSizes.data(),
		1, &destDescHandle, &numDescriptors,
		gxapi::eDescriptorHeapType::CBV_SRV_UAV);

	// update table parameters
	table.committed = false;
	table.reference = space;
}

template <gxapi::eCommandListType Type>
auto RootTableManager<Type>::FindRootTable(int rootSignatureSlot) -> DescriptorTableState& {
	// root table states are already sorted by init
	auto tableIt = std::lower_bound(
		m_rootTableStates.begin(),
		m_rootTableStates.end(),
		rootSignatureSlot,
		[](const DescriptorTableState& table, int slot) { return table.slot < slot; });

	// slot must be valid
	assert(tableIt != m_rootTableStates.end());

	return *tableIt;
}

template <gxapi::eCommandListType Type>
void RootTableManager<Type>::InitRootTables() {
	m_rootTableStates.clear();
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();

	for (size_t slot = 0; slot < desc.rootParameters.size(); slot++) {
		auto& param = desc.rootParameters[slot];
		if (param.type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
			auto& ranges = param.As<gxapi::RootParameterDesc::DESCRIPTOR_TABLE>().ranges;

			if (ranges.size() <= 0) {
				continue;
			}

			// if first range is NOT a sampler, non of the ranges are
			// dynamic samplers are not supported, thus the exception
			if (ranges[0].type == gxapi::DescriptorRange::eType::SAMPLER) {
				throw std::runtime_error("Dynamic Samplers are not supported yet.");
			}

			// check if ranges are contiguous and not unbounded
			size_t descriptorCountTotal = 0;
			size_t appendIndex = 0;
			size_t largestIndex = 0;
			for (const auto& range : ranges) {
				size_t rangeOffset;
				descriptorCountTotal += range.numDescriptors;
				if (range.offsetFromTableStart == gxapi::DescriptorRange::OFFSET_APPEND) {
					rangeOffset = appendIndex;
				}
				else {
					rangeOffset = range.offsetFromTableStart;
				}
				largestIndex = std::max(largestIndex, rangeOffset + range.numDescriptors);
				appendIndex = rangeOffset + range.numDescriptors;
			}
			assert(descriptorCountTotal == largestIndex);

			// add record for this table
			m_rootTableStates.push_back({ m_heap->Allocate((uint32_t)descriptorCountTotal), (int)slot });
			SetRootDescriptorTable(m_commandList, m_rootTableStates.back().slot, m_rootTableStates.back().reference.Get(0));
			m_rootTableStates.back().bindings.resize(descriptorCountTotal);
		}
	}
}

template <gxapi::eCommandListType Type>
void RootTableManager<Type>::CommitRootTables() {
	for (auto& table : m_rootTableStates) {
		table.committed = true;
	}
}

template <gxapi::eCommandListType Type>
void RootTableManager<Type>::RenewRootTables() {
	for (auto& table : m_rootTableStates) {
		DuplicateRootTable(table);
	}
}

template <gxapi::eCommandListType Type>
void RootTableManager<Type>::SetRootDescriptorTable(gxapi::IGraphicsCommandList* list, unsigned parameterIndex, gxapi::DescriptorHandle baseHandle) {
	list->SetGraphicsRootDescriptorTable(parameterIndex, baseHandle);
}

template <gxapi::eCommandListType Type>
void RootTableManager<Type>::SetRootDescriptorTable(gxapi::IComputeCommandList* list, unsigned parameterIndex, gxapi::DescriptorHandle baseHandle) {
	list->SetComputeRootDescriptorTable(parameterIndex, baseHandle);
}

template <gxapi::eCommandListType Type>
void RootTableManager<Type>::SetRootSignature(gxapi::IGraphicsCommandList* list, gxapi::IRootSignature* sig) {
	list->SetGraphicsRootSignature(sig);
}

template <gxapi::eCommandListType Type>
void RootTableManager<Type>::SetRootSignature(gxapi::IComputeCommandList* list, gxapi::IRootSignature* sig) {
	list->SetComputeRootSignature(sig);
}



} // namespace inl::gxeng