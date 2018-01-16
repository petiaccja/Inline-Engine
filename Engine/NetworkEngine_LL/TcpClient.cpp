#include "TcpClient.hpp"
#include "TcpSocketBuilder.hpp"

namespace inl::net::sockets
{
	TcpClient::TcpClient(Socket *soc)
	{
		m_socket = std::unique_ptr<Socket>(soc); // will this work
	}

	TcpClient::TcpClient(SocketProtocol protocol)
	{
		m_socket = TcpSocketBuilder().AsNonBlocking().AsReusable().Protocol(protocol).Build();
	}

	bool TcpClient::Connect(const IPAddress& addrStr) 
	{ 
		return m_socket->Connect(addrStr); 
	}

	bool TcpClient::Close() const 
	{ 
		return m_socket->Close(); 
	}

	bool TcpClient::HasPendingData(uint32_t& pendingDataSize) const 
	{ 
		return m_socket->HasPendingData(pendingDataSize); 
	}

	bool TcpClient::Send(const uint8_t* data, int32_t count, int32_t& sent) const 
	{ 
		return m_socket->Send(data, count, sent); 
	}

	bool TcpClient::Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags) const 
	{ 
		return m_socket->Recv(data, size, read, flags); 
	}

	bool TcpClient::Wait(SocketWaitConditions cond, std::chrono::milliseconds t) const 
	{ 
		return m_socket->Wait(cond, t); 
	}

	SocketConnectionState TcpClient::GetConnectionState() const 
	{ 
		return m_socket->GetConnectionState(); 
	}

	void TcpClient::GetAddress(IPAddress& outAddr) const 
	{ 
		return m_socket->GetAddress(outAddr); 
	}

	int32_t TcpClient::GetPort() const 
	{ 
		return m_socket->GetPort(); 
	}
}