#pragma once

#include "Socket.hpp"

#include <memory>

namespace inl::net::sockets
{
	class UdpSocket
	{
	public:
		UdpSocket(Socket *soc);
		UdpSocket(SocketProtocol protocol = SocketProtocol::IPv4);

		bool Bind(const IPAddress &addr);
		bool SendTo(const uint8_t* data, int32_t count, int32_t& sent, const IPAddress& addrDest);
		bool RecvFrom(uint8_t* data, int32_t size, int32_t& read, IPAddress& srcAddr, SocketReceiveFlags flags = SocketReceiveFlags::None);
		bool GetPeerAddress(IPAddress& outAddr);
		bool JoinMulticastGroup(const IPAddress& addrStr);
		bool LeaveMulticastGroup(const IPAddress& addrStr);
		bool SetMulticastLoopback(bool loopback);
		bool SetMulticastTtl(uint8_t timeToLive);
		uint32_t GetPort();
		bool SetReuseAddr(bool allowReuse = true);

	private:
		std::unique_ptr<Socket> m_socket;
	};
}