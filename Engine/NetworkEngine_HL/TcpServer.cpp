#include "TcpServer.hpp"

namespace inl::net::servers
{
	TcpServer::TcpServer(uint32_t max_connections, uint16_t port)
		: m_maxConnections(max_connections)
		, m_port(port)
		, m_run(false)
	{
		listener = TcpSocketBuilder().AsReusable().Bind(IPAddress(0, 0, 0, 0, port)).Listening().BuildListener();
		m_connectionPool.SetMaxConnections(max_connections);
	}

	void TcpServer::Start()
	{
		m_run = true;
		std::thread acceptor_thread(&TcpServer::BeginAccept, this);
		m_acceptingThread.swap(acceptor_thread);
	}

	void TcpServer::BeginAccept()
	{
		while (m_run)
		{
			TcpClient *c = listener->AcceptClient();
			ServerConnection *connection = new ServerConnection(c);
			m_connectionPool.Add(connection); // maybe i should thread the add fn in the handler
		}
	}

	void TcpServer::EndAccept()
	{
		m_run = false;
	}
}