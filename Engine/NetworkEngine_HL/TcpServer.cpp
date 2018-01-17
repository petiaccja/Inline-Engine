#include "TcpServer.hpp"

#include <BaseLibrary/Exception/Exception.hpp>

#include "TcpConnection.hpp"
#include "NetworkEngine_LL/TcpSocketBuilder.hpp"
#include "NetworkEngine_LL/TcpClient.hpp"

namespace inl::net::servers
{
	TcpServer::TcpServer(uint32_t max_connections, uint16_t port)
		: m_maxConnections(max_connections)
		, m_port(port)
		, m_run(false)
	{
		m_connectionHandler = std::make_unique<TcpConnectionHandler>();
		if (max_connections == 0 || port == 0)
			throw InvalidArgumentException("TcpServer::TcpServer()");

		listener = TcpSocketBuilder().AsReusable().Bind(IPAddress(0, 0, 0, 0, port)).Listening().BuildListener();
		m_connectionHandler->SetMaxConnections(max_connections);
	}

	void TcpServer::Start()
	{
		m_run = true;
		m_connectionHandler->Start();

		std::thread receive_thread(&TcpServer::AcceptClients, this);
		m_acceptingThread.swap(receive_thread);
	}

	void TcpServer::Stop()
	{
		m_run.exchange(false);
	}

	void TcpServer::AcceptClients()
	{
		while (m_run.load())
		{
			TcpClient *c = listener->AcceptClient();
			if (c)
			{
				std::shared_ptr<TcpConnection> connection = std::make_shared<TcpConnection>(c);
				m_connectionHandler->AddClient(connection); // maybe i should thread the add fn in the handler
			}
		}
	}
}