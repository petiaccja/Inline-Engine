#pragma once

#include <NetworkEngine_LL/Net.hpp>

#include <memory>

namespace inl::net
{
	namespace servers
	{
		class TcpServer;
	}

	class MessageQueue;

	class Server
	{
	public:
		Server(uint32_t max_connections = 20, uint16_t port = DEFAULT_SERVER_PORT);

		void Start();
		void Stop();

	private:
		std::shared_ptr<inl::net::servers::TcpServer> m_tcpServer;

		std::shared_ptr<MessageQueue> m_queue;
	};
}