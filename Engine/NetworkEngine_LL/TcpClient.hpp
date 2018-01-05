#pragma once

#include "TcpSocketBuilder.hpp"

namespace inl::net::sockets
{
	class TcpClient
	{
	public:
		inline TcpClient(Socket *soc)
		{
			m_socket = std::unique_ptr<Socket>(soc); // will this work
		}

		inline TcpClient(SocketProtocol protocol = SocketProtocol::IPv4)
		{
			m_socket = TcpSocketBuilder().AsNonBlocking().AsReusable().Protocol(protocol).Build();
		}

		inline ~TcpClient()
		{
			Close();
		}

		inline bool Connect(const IPAddress& addrStr) { return m_socket->Connect(addrStr); }
		inline bool Close() const { return m_socket->Close(); }
		inline bool HasPendingData(uint32_t& pendingDataSize) const { return m_socket->HasPendingData(pendingDataSize); }
		inline bool Send(const uint8_t* data, int32_t count, int32_t& sent) const { return m_socket->Send(data, count, sent); }
		inline bool Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags = SocketReceiveFlags::None) const { return m_socket->Recv(data, size, read, flags); }
		inline bool Wait(SocketWaitConditions cond, std::chrono::milliseconds t) const { return m_socket->Wait(cond, t); }
		inline SocketConnectionState GetConnectionState() const { return m_socket->GetConnectionState(); }
		inline void GetAddress(IPAddress& outAddr) const { return m_socket->GetAddress(outAddr); }
		inline int32_t GetPortNo() const { return m_socket->GetPortNo(); }

	private:
		std::unique_ptr<Socket> m_socket;
	};
}