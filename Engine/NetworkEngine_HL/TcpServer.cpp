#include "TcpServer.hpp"

#include <Exception/Exception.hpp>

namespace inl::net::servers
{
	TcpServer::TcpServer(uint32_t max_connections, uint16_t port)
		: m_maxConnections(max_connections)
		, m_port(port)
		, m_run(false)
	{
		if (max_connections == 0 || port == 0)
			throw InvalidArgumentException("TcpServer::TcpServer()");

		listener = TcpSocketBuilder().AsReusable().Bind(IPAddress(0, 0, 0, 0, port)).Listening().BuildListener();
		m_connectionHandler.SetMaxConnections(max_connections);
	}

	void TcpServer::Start()
	{
		if (!m_run.load())
		{
			m_run = true;
			std::thread acceptor_thread(&TcpServer::BeginAccept, this);
			m_acceptingThread.swap(acceptor_thread);
		}
		else
		{
			// already running
		}
	}

	void TcpServer::BeginAccept()
	{
		while (m_run.load())
		{
			TcpClient *c = listener->AcceptClient();
			if (c)
			{
				ServerConnection *connection = new ServerConnection(c);
				m_connectionHandler.Add(connection); // maybe i should thread the add fn in the handler
			}
		}
	}

	void TcpServer::EndAccept()
	{
		m_run = false;
	}
}