#pragma once

#include <thread>
#include <queue>

#include <SpinMutex.hpp>

#include "TcpSocketBuilder.hpp"

namespace inl::net::sockets
{
	class TcpListener
	{
	public:
		inline TcpListener(uint16_t port, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1))
			: m_port(port)
			, m_sleepTime(inSleepTime)
			, m_socket(nullptr)
		{
		}

		inline TcpListener(Socket *InSocket, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1))
			: m_sleepTime(inSleepTime)
			, m_socket(InSocket)
		{
		}

		inline ~TcpListener()
		{
			delete m_socket;
			m_socket = nullptr;
		}

		inline Socket* GetSocket() const { return m_socket; }
		TcpClient *AcceptClient();

	private:
		std::chrono::milliseconds m_sleepTime;
		Socket* m_socket;
		uint16_t m_port;
	};
}