#pragma once

#include "TcpListener.hpp"

#include "ServerConnectionHandler.hpp"

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
		TcpListener *listener;
		uint32_t m_maxConnections;
		uint16_t m_port;

		std::atomic_bool m_run;

		std::thread m_acceptingThread;

		ServerConnectionHandler *m_connectionHandler;
	};
}