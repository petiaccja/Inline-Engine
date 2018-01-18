#pragma once

#include "NetworkEngine_LL/TcpListener.hpp"

#include "TcpConnectionHandler.hpp"

namespace inl::net
{
	class Server;
}

namespace inl::net::servers
{
	using namespace sockets;

	class TcpServer
	{
		friend class inl::net::Server;

	public:
		TcpServer(uint32_t max_connections, uint16_t port = DEFAULT_SERVER_PORT);

		void Start();
		void Stop();

	private:
		void AcceptClients();

	private:
		std::unique_ptr<TcpListener> listener;
		uint32_t m_maxConnections;
		uint16_t m_port;

		std::atomic_bool m_run;

		std::thread m_acceptingThread;

		std::unique_ptr<TcpConnectionHandler> m_connectionHandler;
	};
}