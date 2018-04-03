#include "Mutex.hpp"


namespace inl::jobs {


//------------------------------------------------------------------------------
// Awaiter
//------------------------------------------------------------------------------

Mutex::MutexAwaiter::MutexAwaiter(Mutex& mtx, Fence::FenceAwaiter fenceAwaiter)
	: m_mtx(mtx), m_fenceAwaiter(fenceAwaiter)
{}


bool Mutex::MutexAwaiter::await_ready() const noexcept {
	return m_fenceAwaiter.await_ready();
}


Mutex& Mutex::MutexAwaiter::await_resume() noexcept {
	m_fenceAwaiter.await_resume();
	m_mtx.m_ignoreNext = true;
	return m_mtx;
}



//------------------------------------------------------------------------------
// Mutex
//------------------------------------------------------------------------------
Mutex::Mutex() {
	m_waitFor.store(0);
	m_unlockSignal.store(0);
}


Mutex::MutexAwaiter Mutex::operator co_await() {
	uint64_t waitFor = m_waitFor.fetch_add(1, std::memory_order_acq_rel);
	auto fenceWait = m_fence.Wait(waitFor);
	return MutexAwaiter(*this, fenceWait);
}


void Mutex::lock() {
	if (m_ignoreNext) {
		return;
	}
	uint64_t waitFor = m_waitFor.fetch_add(1, std::memory_order_acq_rel);
	m_fence.WaitExplicit(waitFor);
}


bool Mutex::try_lock() {
	uint64_t unlockSignal = m_unlockSignal.load(std::memory_order_acquire);
	bool locked = m_waitFor.compare_exchange_strong(unlockSignal, unlockSignal+1);
	return locked;
}


void Mutex::unlock() {
	m_ignoreNext = false;
	uint64_t unlockSignal = m_unlockSignal.fetch_add(1, std::memory_order_relaxed);
	m_fence.Signal(unlockSignal + 1);
}


} // namespace inl::jobs