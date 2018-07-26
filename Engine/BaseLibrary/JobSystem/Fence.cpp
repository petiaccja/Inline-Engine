#include "Fence.hpp"
#include "Scheduler.hpp"

#include <mutex>
#include <future>
#include "BaseLibrary/AtScopeExit.hpp"


namespace inl::jobs {


bool Fence::FenceAwaiter::await_ready() const noexcept {
	return m_fence.m_currentValue >= m_targetValue;
}


Fence::FenceAwaiter::FenceAwaiter(const Fence& f, uint64_t expected) noexcept
	: m_fence(f), m_targetValue(expected), m_next(nullptr), m_awaitingHandle(nullptr)
{
	volatile int a;
	a = 0;
}

bool Fence::FenceAwaiter::await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine, Scheduler* scheduler) noexcept {
	m_awaitingHandle = awaitingCoroutine;
	m_scheduler = scheduler;

	std::lock_guard<SpinMutex> lkg(m_fence.m_mtx);

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
	std::unique_lock<SpinMutex> lk(m_mtx);

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


	int total = 0, awoke = 0, putback = 0;
	bool totalCounted = false;
	AtScopeExit([&]{
		assert(total == awoke + putback);
	});
	// Process the list.
	do {
		FenceAwaiter* unsatisfied = nullptr;
		while (list != nullptr) {
			if (!totalCounted) {
				++total;
			}
			FenceAwaiter* next = list->m_next;
			uint64_t expected = list->m_targetValue;


			if (expected <= newCurrentValue) {
				++awoke;
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
		totalCounted = true;

		// See if somebody changed the fence values and re-process list.
		if (!unsatisfied) {
			return;
		}
		list = unsatisfied;
		lk.lock();
		newCurrentValue = m_currentValue;
		if (currentValue == newCurrentValue) {
			// Put remaining in list back.
			while (list != nullptr) {
				++putback;
				list->m_next = m_firstAwaiter;
				m_firstAwaiter = list;
				list = list->m_next;
			}
			return;
		}
		currentValue = newCurrentValue;
		lk.unlock();
	} while (true);
}

Fence::FenceAwaiter Fence::Wait(uint64_t value) const {
	return FenceAwaiter{*this, value};
}

bool Fence::TryWait(uint64_t value) const {
	std::lock_guard<SpinMutex> lkg(m_mtx);
	return m_currentValue >= value;
}


void Fence::WaitExplicit(uint64_t value) const {
	std::future<void> fut = [value, this]() -> std::future<void> {
		co_await Wait(value);
	}();
	fut.wait();
}


} // namespace inl::jobs