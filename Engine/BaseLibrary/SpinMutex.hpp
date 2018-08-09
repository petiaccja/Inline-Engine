#pragma once

#include <atomic>
#include <thread>


namespace inl {


class SpinMutex {
public:
	SpinMutex();
	SpinMutex(const SpinMutex&) = delete;
	SpinMutex& operator=(const SpinMutex&) = delete;
	SpinMutex(SpinMutex&&) noexcept;
	SpinMutex& operator=(SpinMutex&&) noexcept;

	void lock();
	bool try_lock();
	void unlock();

	void native_handle() = delete;

private:
	std::atomic<std::thread::id> ownerId;
	std::atomic_bool flag;
};


} // namespace inl
