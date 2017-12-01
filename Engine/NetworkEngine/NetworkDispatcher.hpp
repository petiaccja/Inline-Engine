#pragma once

#include <queue>
#include <mutex>
#include <atomic>

class SpinLock {
	std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
	void lock() 
	{
		while (locked.test_and_set(std::memory_order_acquire)) 
		{ 
			; 
		}
	}
	void unlock() 
	{
		locked.clear(std::memory_order_release);
	}
};

class NetworkDispatcher
{
public:
	NetworkDispatcher()
	{
	}

	void Enqueue(std::string &job)
	{
		m_lock.lock();
		m_jobs.push(job);
		m_lock.unlock();
	}

	std::string &Dequeue()
	{
		m_lock.lock();
		std::string job = m_jobs.front();
		m_jobs.pop();
		m_lock.unlock();
		return job;
	}
private:
	std::queue<std::string> m_jobs;
	SpinLock m_lock;
};