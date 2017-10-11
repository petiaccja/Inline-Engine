#pragma once


#include "Delegate.hpp"
#include <set>
#include "SpinMutex.hpp"
#include <mutex>


namespace inl {



template <class... ArgsT>
class Event {
public:
	void operator()(ArgsT... args) {
		std::lock_guard<decltype(m_mtx)> lkg(m_mtx);
		for (auto& del : m_delegates) {
			del(args...);
		}
	}
	
	void operator+=(const Delegate<void(ArgsT...)>& func) {
		std::lock_guard<decltype(m_mtx)> lkg(m_mtx);

		m_delegates.insert(func);
	}
	bool operator-=(const Delegate<void(ArgsT...)>& func) {
		std::lock_guard<decltype(m_mtx)> lkg(m_mtx);

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
	spin_mutex m_mtx;
	std::multiset<Delegate<void(ArgsT...)>> m_delegates;
};



} // namespace inl