#pragma once

#include "MessageQueue.hpp"
#include "TcpServer.hpp"
//#include "UdpServer.hpp

namespace inl::net
{
	using namespace servers;

	class Server
	{
	public:
		Server(uint32_t max_connections = 20, uint16_t port = DEFAULT_SERVER_PORT);

		void Start();
		void Stop();

	private:
		std::unique_ptr<TcpServer> m_tcpServer;

		std::shared_ptr<MessageQueue> m_queue;
	};
}