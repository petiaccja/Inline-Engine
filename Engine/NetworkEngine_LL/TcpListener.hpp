#pragma once

#include <thread>
#include <queue>

#include "Socket.hpp"

namespace inl::net::servers
{
	class TcpConnectionHandler;
}

namespace inl::net::sockets
{
	class TcpClient;

	class TcpListener
	{
		friend class inl::net::servers::TcpConnectionHandler;

	public:
		TcpListener(uint16_t port, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1));
		TcpListener(Socket *InSocket, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1));

		//inline std::shared_ptr<Socket> GetSocket() const { return m_socket; }
		TcpClient *AcceptClient();

	private:
		std::chrono::milliseconds m_sleepTime;
		std::unique_ptr<Socket> m_socket;
		uint16_t m_port;
	};
}