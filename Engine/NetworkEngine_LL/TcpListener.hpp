#pragma once

#include <thread>
#include <queue>

#include "TcpSocketBuilder.hpp"

namespace inl::net::sockets
{
	class TcpListener
	{
	public:
		inline TcpListener(uint16_t port, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1))
			: m_port(port)
			, m_sleepTime(inSleepTime)
		{
			m_socket = std::unique_ptr<Socket>(&(*TcpSocketBuilder().AsNonBlocking().AsReusable().Bind(IPAddress(0, 0, 0, 0, port)).Listening().Build()));
		}

		inline TcpListener(Socket *InSocket, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1))
			: m_sleepTime(inSleepTime)
		{
			m_socket = std::unique_ptr<Socket>(InSocket);
		}

		inline ~TcpListener()
		{
		}

		//inline std::shared_ptr<Socket> GetSocket() const { return m_socket; }
		std::unique_ptr<TcpClient> AcceptClient();

	private:
		std::chrono::milliseconds m_sleepTime;
		std::unique_ptr<Socket> m_socket;
		uint16_t m_port;
	};
}