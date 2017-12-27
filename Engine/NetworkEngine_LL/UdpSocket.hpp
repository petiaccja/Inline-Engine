#pragma once

#include "Socket.hpp"

namespace inl::net::sockets
{
	class UdpSocket
	{
	public:
		UdpSocket(SocketProtocol protocol = SocketProtocol::IPv4)
		{
			m_socket = new Socket(SocketType::Datagram, protocol);
		}

		inline bool Bind(const IPAddress &addr) { return m_socket->Bind(addr); }
		inline bool SendTo(const uint8_t* data, int32_t count, int32_t& sent, const IPAddress& addrDest) { return m_socket->SendTo(data, count, sent, addrDest); }
		inline bool RecvFrom(uint8_t* data, int32_t size, int32_t& read, IPAddress& srcAddr, SocketReceiveFlags flags = SocketReceiveFlags::None) { return m_socket->RecvFrom(data, size, read, srcAddr, flags); }
		inline bool GetPeerAddress(IPAddress& outAddr) { return m_socket->GetPeerAddress(outAddr); }
		inline bool JoinMulticastGroup(const IPAddress& addrStr) { return m_socket->JoinMulticastGroup(addrStr); }
		inline bool LeaveMulticastGroup(const IPAddress& addrStr) { return m_socket->LeaveMulticastGroup(addrStr); }
		inline bool SetMulticastLoopback(bool loopback) { return m_socket->SetMulticastLoopback(loopback); }
		inline bool SetMulticastTtl(uint8_t timeToLive) { return m_socket->SetMulticastTtl(timeToLive); }
		inline int32_t GetPortNo() { return m_socket->GetPortNo(); }
		inline bool SetReuseAddr(bool allowReuse = true) { m_socket->SetReuseAddr(allowReuse); }

	private:
		Socket * m_socket;
	};
}