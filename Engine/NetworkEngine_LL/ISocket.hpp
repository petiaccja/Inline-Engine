#pragma once

#include <string>
#include <chrono>
#include <memory>

#include "Net.hpp"
#include "IPAddress.hpp"
#include "Enums.hpp"

namespace inl::net::sockets
{
	using namespace enums;

	class Socket;

	class ISocket
	{
	public:
		inline ISocket()
			: m_socketType(SocketType::Unknown)
			, m_protocol(SocketProtocol::IPv4)
		{
		}

		inline ISocket(SocketType InSocketType, SocketProtocol protocol = SocketProtocol::IPv4)
			: m_socketType(InSocketType)
			, m_protocol(protocol)
		{
		}

		inline virtual ~ISocket()
		{
		}

		virtual bool Close() = 0;
		virtual bool Bind(const IPAddress &addr) = 0;
		virtual bool Connect(const IPAddress& addr) = 0;
		virtual bool Listen() = 0;
		virtual bool WaitForPendingConnection(bool& hasPendingConnection, std::chrono::milliseconds t) = 0;
		virtual bool HasPendingData(uint32_t& pendingDataSize) = 0;
		virtual std::unique_ptr<Socket> Accept() = 0;
		virtual bool SendTo(const uint8_t* data, int32_t count, int32_t& sent, const IPAddress& addrDest) = 0;
		virtual bool Send(const uint8_t* data, int32_t count, int32_t& sent) = 0;
		virtual bool RecvFrom(uint8_t* data, int32_t size, int32_t& read, IPAddress& srcAddr, SocketReceiveFlags flags = SocketReceiveFlags::None) = 0;
		virtual bool Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags = SocketReceiveFlags::None) = 0;
		virtual bool Wait(SocketWaitConditions cond, std::chrono::milliseconds t) = 0;
		virtual SocketConnectionState GetConnectionState() = 0;
		virtual void GetAddress(IPAddress& outAddr) = 0;
		virtual bool GetPeerAddress(IPAddress& outAddr) = 0;
		virtual bool SetNonBlocking(bool isNonBlocking = true) = 0;
		virtual bool JoinMulticastGroup(const IPAddress& addrStr) = 0;
		virtual bool LeaveMulticastGroup(const IPAddress& addrStr) = 0;
		virtual bool SetMulticastLoopback(bool loopback) = 0;
		virtual bool SetMulticastTtl(uint8_t timeToLive) = 0;
		virtual bool SetReuseAddr(bool allowReuse = true) = 0;
		virtual bool SetLinger(bool shouldLinger = true, int32_t t = 0) = 0;
		virtual bool SetSendBufferSize(int32_t size, int32_t& newSize) = 0;
		virtual bool SetReceiveBufferSize(int32_t size, int32_t& newSize) = 0;
		virtual uint32_t GetPortNo() = 0;

		inline SocketType GetSocketType() const
		{
			return m_socketType;
		}

		inline SocketProtocol GetSocketProtocol() const
		{
			return m_protocol;
		}

	private:
		const SocketType m_socketType;
		const SocketProtocol m_protocol;
	};
}