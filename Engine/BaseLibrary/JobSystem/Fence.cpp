#include "Fence.hpp"
#include "Scheduler.hpp"
#include "Task.hpp"

#include <mutex>
#include <future>


namespace inl::jobs {


bool Fence::FenceAwaiter::await_ready() const noexcept {
	return m_fence.m_currentValue >= m_targetValue;
}


bool Fence::FenceAwaiter::await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler) noexcept {
	m_awaitingHandle = awaitingCoroutine;
	m_scheduler = scheduler;

	std::lock_guard<spin_mutex> lkg(m_fence.m_mtx);

	// Check if condition was satisfied.
	if (m_fence.m_currentValue >= m_targetValue) {
		return false;
	}

	// Add this to the waiting list.
	FenceAwaiter* first = m_fence.m_firstAwaiter;
	m_next = first;
	m_fence.m_firstAwaiter = this;

	return true;
}


Fence::Fence(uint64_t initial) {
	m_currentValue = initial;
	m_firstAwaiter = nullptr;
}

void Fence::Signal(uint64_t value) {
	std::unique_lock<spin_mutex> lk(m_mtx);

	// Increase value and steal list if new value is bigger.
	uint64_t currentValue = m_currentValue;
	if (currentValue > value) {
		return;
	}

	m_currentValue = value;

	FenceAwaiter* list = m_firstAwaiter;
	m_firstAwaiter = nullptr;

	lk.unlock();

	uint64_t newCurrentValue = value;

	// Process the list.
	do {
		FenceAwaiter* unsatisfied = nullptr;
		while (list != nullptr) {
			FenceAwaiter* next = list->m_next;
			uint64_t expected = list->m_targetValue;


			if (expected <= newCurrentValue) {
				// Resume coroutine if expected value is satisfied.
				if (list->m_scheduler) {
					list->m_scheduler->Resume(list->m_awaitingHandle);
				}
				else {
					list->m_awaitingHandle.resume();
				}
			}
			else {
				// Put it back into unsatisfied list.
				list->m_next = unsatisfied;
				unsatisfied = list;
			}

			list = next;
		}

		// See if somebody changed the fence values and re-process list.
		list = unsatisfied;
		lk.lock();
		newCurrentValue = m_currentValue;
		if (currentValue == newCurrentValue || list == nullptr) {
			// Put remaining in list back.
			while (list != nullptr) {
				list->m_next = m_firstAwaiter;
				m_firstAwaiter = list;
			}
			return;
		}
		lk.unlock();
	} while (true);
}

Fence::FenceAwaiter Fence::Wait(uint64_t value) const {
	return FenceAwaiter{*this, value};
}

bool Fence::TryWait(uint64_t value) const {
	std::lock_guard<spin_mutex> lkg(m_mtx);
	return m_currentValue >= value;
}


void Fence::WaitExplicit(uint64_t value) const {
	std::future<void> fut = [value, this]() -> std::future<void> {
		co_await this->Wait(value);
	}();
	fut.wait();
}


} // namespace inl::jobs