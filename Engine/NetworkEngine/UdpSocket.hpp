#pragma once

#include "Socket.hpp"

class UdpSocket
{
public:
	UdpSocket(SocketProtocol protocol = SocketProtocol::IPv4)
	{
		m_socket = new Socket(SocketType::Datagram, protocol); // should i make a udp socket builder?
	}

	inline bool Bind(const IPAddress &addr) { return m_socket->Bind(addr); }
	inline bool HasPendingData(uint32_t& pendingDataSize) { return m_socket->HasPendingData(pendingDataSize); } // can i have this here?
	inline bool SendTo(const uint8_t* data, int32_t count, int32_t& sent, const IPAddress& addrDest) { return m_socket->SendTo(data, count, sent, addrDest); }
	inline bool RecvFrom(uint8_t* data, int32_t size, int32_t& read, IPAddress& srcAddr, SocketReceiveFlags flags = SocketReceiveFlags::None) { return m_socket->RecvFrom(data, size, read, srcAddr, flags); }
	inline bool Wait(SocketWaitConditions cond, std::chrono::milliseconds t) { return m_socket->Wait(cond, t); } // can i have this here?
	inline bool GetPeerAddress(IPAddress& outAddr) { return m_socket->GetPeerAddress(outAddr); }
	inline bool JoinMulticastGroup(const IPAddress& addrStr) { return m_socket->JoinMulticastGroup(addrStr); }
	inline bool LeaveMulticastGroup(const IPAddress& addrStr) { return m_socket->LeaveMulticastGroup(addrStr); }
	inline bool SetMulticastLoopback(bool loopback) { return m_socket->SetMulticastLoopback(loopback); }
	inline bool SetMulticastTtl(uint8_t timeToLive) { return m_socket->SetMulticastTtl(timeToLive); }
	inline int32_t GetPortNo() { return m_socket->GetPortNo(); }

private:
	Socket *m_socket;
};