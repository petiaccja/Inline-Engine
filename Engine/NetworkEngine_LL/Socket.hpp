#pragma once

#include "ISocket.hpp"
#include "Enums.hpp"

namespace inl::net::sockets
{
	class Socket : public ISocket
	{
	public:
		inline Socket(SocketType socketType, SocketProtocol protocol = SocketProtocol::IPv4)
			: ISocket(socketType, protocol)
		{
			init();
		}

		inline Socket(SOCKET newSocket, SocketType socketType, SocketProtocol protocol = SocketProtocol::IPv4)
			: ISocket(socketType, protocol)
			, m_socket(newSocket)
		{
			init();
		}

		virtual ~Socket() { Close(); }

	public:

		// ISocket overrides

		virtual bool Close() override;
		virtual bool Bind(const IPAddress &addr) override;
		virtual bool Connect(const IPAddress& addrStr) override;
		inline virtual bool Listen() override { return listen(m_socket, SOMAXCONN) == 0; }
		virtual bool WaitForPendingConnection(bool& hasPendingConnection, std::chrono::milliseconds t) override;
		virtual bool HasPendingData(uint32_t& pendingDataSize) override;
		virtual std::unique_ptr<Socket> Accept() override;
		virtual bool SendTo(const uint8_t* data, int32_t count, int32_t& sent, const IPAddress& addrDest) override;
		virtual bool Send(const uint8_t* data, int32_t count, int32_t& sent) override;
		virtual bool RecvFrom(uint8_t* data, int32_t size, int32_t& read, IPAddress& srcAddr, SocketReceiveFlags flags = SocketReceiveFlags::None) override;
		virtual bool Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags = SocketReceiveFlags::None) override;
		virtual bool Wait(SocketWaitConditions cond, std::chrono::milliseconds t) override;
		virtual SocketConnectionState GetConnectionState() override;
		virtual void GetAddress(IPAddress& outAddr) override;
		virtual bool GetPeerAddress(IPAddress& outAddr) override;
		virtual bool SetNonBlocking(bool isNonBlocking = true) override;

		virtual bool JoinMulticastGroup(const IPAddress& addrStr) override;
		virtual bool LeaveMulticastGroup(const IPAddress& addrStr) override;
		inline virtual bool SetMulticastLoopback(bool loopback) override { return (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopback, sizeof(loopback)) == 0); }
		inline virtual bool SetMulticastTtl(uint8_t timeToLive) override { return (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&timeToLive, sizeof(timeToLive)) == 0); }
		inline virtual bool SetReuseAddr(bool allowReuse = true) override
		{
			int param = allowReuse ? 1 : 0;
			return setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&param, sizeof(param)) == 0;
		}

		virtual bool SetLinger(bool shouldLinger = true, int32_t t = 0) override;
		virtual bool SetSendBufferSize(int32_t size, int32_t& newSize) override;
		virtual bool SetReceiveBufferSize(int32_t size, int32_t& newSize) override;
		virtual uint32_t GetPort() override;

	private:
		SOCKET getNativeSocket() { return m_socket; }
		void init();

		virtual SocketReturn HasState(SocketParam state, std::chrono::milliseconds t = std::chrono::milliseconds(0));
		virtual SocketErrors TranslateErrorCode(int32_t code);
		virtual int TranslateFlags(SocketReceiveFlags flags);

		inline void UpdateActivity()
		{
			m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();
		}

	private:
		SOCKET m_socket = INVALID_SOCKET;

		long long m_lastActivityTime;
	};
}