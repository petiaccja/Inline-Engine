#pragma once

#include "TcpListener.hpp"

#include "TcpConnectionHandler.hpp"

namespace inl::net::servers
{
	using namespace sockets;

	class TcpServer
	{
	public:
		TcpServer(uint32_t max_connections, uint16_t port);

		void Start();

	private:
		void BeginAccept();
		void EndAccept();

	private:
		std::unique_ptr<TcpListener> listener;
		uint32_t m_maxConnections;
		uint16_t m_port;

		std::atomic_bool m_run;

		std::thread m_acceptingThread;

		std::unique_ptr<TcpConnectionHandler> m_connectionHandler;
	};
}