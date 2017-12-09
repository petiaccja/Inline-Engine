#pragma once

#include "TcpSocketBuilder.hpp"

class TcpClient
{
public:
	inline TcpClient(SocketProtocol protocol = SocketProtocol::IPv4) 
		: m_socket(TcpSocketBuilder().AsNonBlocking().AsReusable().Protocol(protocol).Build())
	{
	}

	inline ~TcpClient()
	{
		Close();
		delete m_socket;
		m_socket = nullptr;
	}

	inline bool Connect(const IPAddress& addrStr) { return m_socket->Connect(addrStr); }
	inline bool Close() { return m_socket->Close(); }
	inline bool HasPendingData(uint32_t& pendingDataSize) { return m_socket->HasPendingData(pendingDataSize); }
	inline bool Send(const uint8_t* data, int32_t count, int32_t& sent) { return m_socket->Send(data, count, sent); }
	inline bool Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags = SocketReceiveFlags::None) { return m_socket->Recv(data, size, read, flags); }
	inline bool Wait(SocketWaitConditions cond, std::chrono::milliseconds t) { return m_socket->Wait(cond, t); }
	inline SocketConnectionState GetConnectionState() { return m_socket->GetConnectionState(); }
	inline void GetAddress(IPAddress& outAddr) { return m_socket->GetAddress(outAddr); }
	inline int32_t GetPortNo() { return m_socket->GetPortNo(); }

private:
	Socket *m_socket;
};