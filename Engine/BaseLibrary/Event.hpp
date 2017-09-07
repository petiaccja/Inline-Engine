#pragma once


#include "Delegate.hpp"
#include <set>


namespace inl {



template <class... ArgsT>
class Event {
public:
	void operator()(ArgsT... args) {
		for (auto& del : m_delegates) {
			del(args...);
		}
	}
	
	void operator+=(const Delegate<void(ArgsT...)>& func) {
		m_delegates.insert(func);
	}
	bool operator-=(const Delegate<void(ArgsT...)>& func) {
		auto it = m_delegates.find(func);
		if (it == m_delegates.end()) {
			return false;
		}
		else {
			m_delegates.erase(it);
			return true;
		}
	}
private:
	std::multiset<Delegate<void(ArgsT...)>> m_delegates;
};



} // namespace inl