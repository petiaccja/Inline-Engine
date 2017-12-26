#pragma once


#include "Delegate.hpp"
#include <set>
#include "SpinMutex.hpp"
#include <mutex>
#include <vector>

namespace inl {



template <class... ArgsT>
class Event {
public:
	void operator()(ArgsT... args) {
		std::unique_lock<spin_mutex> lkg(m_mtx);
		auto delegates = m_delegates;
		auto functions = m_functions;
		lkg.unlock();

		for (auto& del : delegates) {
			del(args...);
		}

		for (auto& fn : functions) {
			fn(args...);
		}
	}
	
	void operator+=(const std::function<void(ArgsT...)>& func) {
		std::lock_guard<decltype(m_mtx)> lkg(m_mtx);

		m_functions.push_back(func);
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

	Event& operator=(const Event& other)
	{
		m_delegates = other.m_delegates;
		m_functions = other.m_functions;

		return *this;
	}

private:
	spin_mutex m_mtx;
	std::multiset<Delegate<void(ArgsT...)>> m_delegates;
	std::vector<std::function<void(ArgsT...)>> m_functions;
};



} // namespace inl