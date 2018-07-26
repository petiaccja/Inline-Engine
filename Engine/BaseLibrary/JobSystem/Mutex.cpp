#include "Mutex.hpp"
#include <cassert>
#include <iostream>


namespace inl::jobs {

//------------------------------------------------------------------------------
// Mutex
//------------------------------------------------------------------------------
Mutex::Mutex() noexcept {
	m_waitFor.store(0);
	m_unlockSignal.store(0);
}


Fence::FenceAwaiter Mutex::Lock() {
	uint64_t waitFor = m_waitFor.fetch_add(1, std::memory_order_acq_rel);
	return m_fence.Wait(waitFor);
}


void Mutex::LockExplicit() {
	uint64_t waitFor = m_waitFor.fetch_add(1, std::memory_order_acq_rel);
	m_fence.WaitExplicit(waitFor);
}


bool Mutex::TryLock() {
	uint64_t unlockSignal = m_unlockSignal.load(std::memory_order_acquire);
	bool locked = m_waitFor.compare_exchange_strong(unlockSignal, unlockSignal+1);
	if (locked) {
		m_dbg_Locked = true;
	}
	return locked;
}


void Mutex::Unlock() {
	//assert(m_dbg_Locked);
	uint64_t unlockSignal = m_unlockSignal.fetch_add(1, std::memory_order_relaxed);
	m_fence.Signal(unlockSignal + 1);
}

} // namespace inl::jobs