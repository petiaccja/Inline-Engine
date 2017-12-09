#pragma once

#include "Socket.hpp"
#include "TcpSocketBuilder.hpp"
#include "NetworkDispatcher.hpp"

#include <thread>

class TcpListener
{
public:
	inline TcpListener(int16_t port, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1))
		: m_port(port)
		, m_sleepTime(inSleepTime)
		, m_socket(nullptr)
		, m_stopping(false)
	{
	}

	inline TcpListener(Socket *InSocket, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1))
		: m_sleepTime(inSleepTime)
		, m_socket(InSocket)
		, m_stopping(false)
	{
	}

	inline ~TcpListener() 
	{ 
		Stop();
		delete m_socket;
		m_socket = nullptr;
	}

	inline Socket* GetSocket() const { return m_socket; }
	inline bool IsActive() const { return ((m_socket != nullptr) && !m_stopping); }
	inline void Stop() { m_stopping = true; }
	void Start();

private:
	void AcceptClients();

private:
	std::chrono::milliseconds m_sleepTime;
	Socket* m_socket;
	bool m_stopping;
	int16_t m_port;

	std::thread m_thread;

	std::function<void(Socket*)> new_connection_event;
};