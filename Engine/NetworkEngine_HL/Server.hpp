#pragma once

#include "TcpServer.hpp"
//#include "UdpServer.hpp"
#include "MessageQueue.hpp"

namespace inl::net
{
	using namespace servers;

	class Server
	{
	public:
		Server(uint32_t max_connections, uint16_t port);

		void Start();
		void Stop();

	private:
		std::unique_ptr<TcpServer> m_tcpServer;

		std::shared_ptr<MessageQueue> m_queue;
	};
}