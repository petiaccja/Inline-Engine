#include "Server.hpp"

#include "MessageQueue.hpp"
#include "TcpServer.hpp"
//#include "UdpServer.hpp

namespace inl::net
{
	Server::Server(uint32_t max_connections, uint16_t port)
	{
		m_tcpServer = std::make_shared<inl::net::servers::TcpServer>(max_connections, port);
		m_queue = std::make_shared<MessageQueue>();
		//m_tcpServer->m_connectionHandler->m_queue = m_queue;
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