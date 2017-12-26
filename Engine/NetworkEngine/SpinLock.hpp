#pragma once

#include <atomic>

class SpinLock 
{
public:
	inline void lock() { while (locked.test_and_set(std::memory_order_acquire)); }
	inline void unlock() { locked.clear(std::memory_order_release); }

private:
	std::atomic_flag locked = ATOMIC_FLAG_INIT;
};