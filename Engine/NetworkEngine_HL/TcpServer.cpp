#include "TcpServer.hpp"

#include <BaseLibrary/Exception/Exception.hpp>

#include "TcpConnection.hpp"
#include "NetworkEngine_LL/TcpSocketBuilder.hpp"
#include "NetworkEngine_LL/TcpClient.hpp"
#include "TcpConnectionHandler.hpp"

namespace inl::net::servers
{
	TcpServer::TcpServer(uint32_t max_connections, uint16_t port)
		: m_maxConnections(max_connections)
		, m_port(port)
		, m_run(false)
	{
		if (max_connections == 0 || port == 0)
			throw InvalidArgumentException("TcpServer::TcpServer()");

		listener = std::shared_ptr<TcpListener>(TcpSocketBuilder().AsReusable().Bind(IPAddress(0, 0, 0, 0, port)).Listening().BuildListener().release());
		m_connectionHandler = std::make_shared<inl::net::servers::TcpConnectionHandler>(listener);
		m_connectionHandler->SetMaxConnections(max_connections);
	}

	void TcpServer::Start()
	{
		m_run = true;
		m_connectionHandler->Start();
	}

	void TcpServer::Stop()
	{
		m_run.exchange(false);
	}
}