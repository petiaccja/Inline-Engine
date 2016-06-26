#pragma once

#include "GraphicsCommandList.hpp"
#include "ComputeCommandList.hpp"
#include "CopyCommandList.hpp"

#include <list>
#include <vector>


namespace inl {
namespace gxeng {


class ExecutionResult {
	struct CommandListRecord {
		BasicCommandList* list;
		float aluVsBandwidthHeavy;
	};
public:
	using Iterator = std::vector<CommandListRecord>::iterator;
	using ConstIterator = std::vector<CommandListRecord>::const_iterator;
public:
	ExecutionResult();
	ExecutionResult(ExecutionResult&& rhs) = default;
	ExecutionResult(const ExecutionResult& rhs) = default;
	ExecutionResult& operator=(ExecutionResult&& rhs) = default;
	ExecutionResult& operator=(const ExecutionResult& rhs) = delete;
	~ExecutionResult();

	void AddCommandList(GraphicsCommandList&& list, float aluVsBandwidthHeavy = 0.5);
	void AddCommandList(ComputeCommandList&& list, float aluVsBandwidthHeavy = 0.5);
	void AddCommandList(CopyCommandList&& list);

	size_t GetNumLists() const { return m_lists.size(); }
	CommandListRecord& operator[](size_t idx) { return m_lists[idx]; }
	const CommandListRecord& operator[](size_t idx) const { return m_lists[idx]; }

	void Reset();

	Iterator Begin();
	Iterator End();
	ConstIterator Begin() const;
	ConstIterator End() const;
	ConstIterator CBegin() const;
	ConstIterator CEnd() const;
private:
	std::vector<CommandListRecord> m_lists;
	std::list<GraphicsCommandList> m_gxLists;
	std::list<ComputeCommandList> m_cuLists;
	std::list<CopyCommandList> m_cpLists;
};


inline ExecutionResult::Iterator begin(ExecutionResult& arg) { return arg.Begin(); }
inline ExecutionResult::Iterator end(ExecutionResult& arg) { return arg.End(); }
inline ExecutionResult::ConstIterator begin(const ExecutionResult& arg) { return arg.Begin(); }
inline ExecutionResult::ConstIterator end(const ExecutionResult& arg) { return arg.End(); }
inline ExecutionResult::ConstIterator cbegin(const ExecutionResult& arg) { return arg.Begin(); }
inline ExecutionResult::ConstIterator cend(const ExecutionResult& arg) { return arg.End(); }


} // namespace gxeng
} // namespace inl