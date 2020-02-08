#pragma once

#include <any>
#include <vector>



template <class... Actions>
struct Visitor {
};


class ActionHeap {
public:
	template <class ActionT>
	void Push(ActionT&& action);

	template <class VisitorT>
	void Visit(VisitorT visitor);

	template <class VisitableT, class VisitorT>
	void Visit(VisitorT visitor);

	void Clear();

private:
	template <class VisitorT, class... Actions>
	void Visit(VisitorT visitor, const Visitor<Actions...>& theSame);

private:
	std::vector<std::any> m_heap;
};


inline void ActionHeap::Clear() {
	m_heap.clear();
}


template <class ActionT>
void ActionHeap::Push(ActionT&& action) {
	m_heap.emplace_back(std::forward<ActionT>(action));
}

template <class VisitorT>
void ActionHeap::Visit(VisitorT visitor) {
	// Must be convertible to a Visitor<...>.
	Visit(visitor, visitor);
}

template <class VisitableT, class VisitorT>
void ActionHeap::Visit(VisitorT visitor) {
	Visit(visitor, std::declval<VisitableT>());
}

template <class VisitorT, class... Actions>
void ActionHeap::Visit(VisitorT visitor, const Visitor<Actions...>& theSame) {
	for (auto& action : m_heap) {
		(..., (action.type() == typeid(Actions) ? void(visitor(std::any_cast<const Actions&>(action))) : void()));
	}
}
