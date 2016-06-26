#include "ExecutionResult.hpp"


namespace inl {
namespace gxeng {


ExecutionResult::ExecutionResult()
	{}


ExecutionResult::~ExecutionResult()
	{}



void ExecutionResult::AddCommandList(GraphicsCommandList&& list, float aluVsBandwidthHeavy) {
	m_gxLists.push_back({ std::move(list) });
	auto it = --m_gxLists.end();
	m_lists.push_back({ &*it, aluVsBandwidthHeavy });
}

void ExecutionResult::AddCommandList(ComputeCommandList&& list, float aluVsBandwidthHeavy) {
	m_cuLists.push_back({ std::move(list) });
	auto it = --m_cuLists.end();
	m_lists.push_back({ &*it, aluVsBandwidthHeavy });
}

void ExecutionResult::AddCommandList(CopyCommandList&& list) {
	m_cpLists.push_back({ std::move(list) });
	auto it = --m_cpLists.end();
	m_lists.push_back({ &*it, -1 });
}


void ExecutionResult::Reset() {
	m_lists.clear();
	m_gxLists.clear();
	m_cuLists.clear();
	m_cpLists.clear();
}


ExecutionResult::Iterator ExecutionResult::Begin() {
	return m_lists.begin();
}

ExecutionResult::Iterator ExecutionResult::End() {
	return m_lists.end();
}

ExecutionResult::ConstIterator ExecutionResult::Begin() const {
	return m_lists.begin();
}

ExecutionResult::ConstIterator ExecutionResult::End() const {
	return m_lists.end();
}

ExecutionResult::ConstIterator ExecutionResult::CBegin() const {
	return m_lists.cbegin();
}

ExecutionResult::ConstIterator ExecutionResult::CEnd() const {
	return m_lists.cend();
}


} // namespace gxeng
} // namespace inl