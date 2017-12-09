#pragma once

#include "SpinLock.hpp"

#include <queue>

template<typename T>
class NetworkDispatcher
{
public:
	inline NetworkDispatcher()
	{
	}

	inline void Enqueue(T &job)
	{
		m_lock.lock();
		m_jobs.push(job);
		m_lock.unlock();
	}

	inline T &Dequeue()
	{
		m_lock.lock();
		T job = m_jobs.front();
		m_jobs.pop();
		m_lock.unlock();
		return job;
	}
private:
	std::queue<T> m_jobs;
	SpinLock m_lock;
};