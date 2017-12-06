#pragma once

#include "ISocket.hpp"
#include "Enums.hpp"

class Socket : public ISocket
{
public:
	inline Socket(SocketType socketType, SocketProtocol protocol = SocketProtocol::IPv4) :
		ISocket(socketType, protocol)
	{
		create();
	}

	inline Socket(SOCKET newSocket, SocketType socketType, SocketProtocol protocol = SocketProtocol::IPv4) :
		ISocket(socketType, protocol),
		m_socket(newSocket)
	{
		create();
	}

	virtual ~Socket()
	{
		Close();
	}

private:
	SOCKET getNativeSocket()
	{
		return m_socket;
	}

	void create();

public:

	// ISocket overrides

	virtual bool Close() override;
	virtual bool Bind(uint16_t port = DEFAULT_SERVER_PORT) override;
	virtual bool Connect(const IPAddress& addrStr) override;
	virtual bool Listen() override;
	virtual bool WaitForPendingConnection(bool& hasPendingConnection, std::chrono::milliseconds t) override;
	virtual bool HasPendingData(uint32_t& pendingDataSize) override;
	virtual class ISocket* Accept() override;
	virtual bool SendTo(const uint8_t* data, int32_t count, int32_t& sent, const IPAddress& addrDest);
	virtual bool Send(const uint8_t* data, int32_t count, int32_t& sent);
	virtual bool RecvFrom(uint8_t* data, int32_t size, int32_t& read, IPAddress& srcAddr, SocketReceiveFlags flags = SocketReceiveFlags::None);
	virtual bool Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags = SocketReceiveFlags::None);
	virtual bool Wait(SocketWaitConditions cond, std::chrono::milliseconds t) override;
	virtual SocketConnectionState GetConnectionState() override;
	virtual void GetAddress(IPAddress& outAddr) override;
	virtual bool GetPeerAddress(IPAddress& outAddr) override;
	virtual bool SetNonBlocking(bool isNonBlocking = true) override;
	virtual bool SetBroadcast(bool allowBroadcast = true) override;
	virtual bool JoinMulticastGroup(const IPAddress& addrStr) override;
	virtual bool LeaveMulticastGroup(const IPAddress& addrStr) override;
	virtual bool SetMulticastLoopback(bool loopback) override;
	virtual bool SetMulticastTtl(uint8_t timeToLive) override;
	virtual bool SetReuseAddr(bool allowReuse = true) override;
	virtual bool SetLinger(bool shouldLinger = true, int32_t t = 0) override;
	virtual bool SetSendBufferSize(int32_t size, int32_t& newSize) override;
	virtual bool SetReceiveBufferSize(int32_t size, int32_t& newSize) override;
	virtual int32_t GetPortNo() override;

protected:
	virtual SocketReturn HasState(SocketParam state, std::chrono::milliseconds t = std::chrono::milliseconds(0));
	virtual SocketErrors TranslateErrorCode(int32_t code);
	virtual int TranslateFlags(SocketReceiveFlags flags);

	inline void UpdateActivity()
	{
		m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();
	}

	SOCKET m_socket;

	long long m_lastActivityTime;
};
