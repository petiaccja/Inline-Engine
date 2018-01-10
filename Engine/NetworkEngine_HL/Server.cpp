#include "Server.hpp"

namespace inl::net
{
	Server::Server(uint32_t max_connections, uint16_t port)
	{
		m_tcpServer = std::make_unique<TcpServer>(max_connections, port);
		m_tcpServer->m_connectionHandler->m_queue = m_queue;
	}

	void Server::Start()
	{
		m_tcpServer->Start();
	}

	void Server::Stop()
	{
		m_tcpServer->Stop();
	}
}