#pragma once

#include <atomic>
#include <thread>


namespace inl {


class spin_mutex {
public:
	spin_mutex();
	spin_mutex(const spin_mutex&) = delete;
	spin_mutex& operator=(const spin_mutex&) = delete;

	void lock();
	bool try_lock();
	void unlock();

	void native_handle() = delete;

private:
	std::atomic<std::thread::id> ownerId;
	std::atomic_bool flag;
};


} // namespace inl
