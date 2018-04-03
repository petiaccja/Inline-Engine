#pragma once

#include <atomic>


namespace inl::jobs {

class spin_mutex {
public:
	spin_mutex() {
		m_flag.clear();
	}
	bool try_lock() {
		bool wasLocked = m_flag.test_and_set();
		return !wasLocked;
	}
	void lock() {
		bool wasLocked = true;
		while (wasLocked) {
			wasLocked = m_flag.test_and_set();
		}
	}
	void unlock() {
		m_flag.clear();
	}
private:
	std::atomic_flag m_flag;
};


} // namespace inl::jobs