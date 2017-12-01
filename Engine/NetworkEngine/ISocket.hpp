#pragma once

#include "Net.hpp"

#include <string>
#include <chrono>

enum SocketType
{
	SOCKTYPE_Unknown,
	SOCKTYPE_Datagram = SOCK_DGRAM,
	SOCKTYPE_Streaming = SOCK_STREAM,
};


enum SocketReceiveFlags
{
	None = 0,
	Peek = 2,
	WaitAll = 0x100,
};

enum SocketWaitConditions
{
	WaitForRead,
	WaitForWrite,
	WaitForReadOrWrite,
};

enum SocketConnectionState
{
	SCS_NotConnected,
	SCS_Connected,
	SCS_ConnectionError,
};

class ISocket
{
protected:
	const SocketType m_socketType;

public:
	inline ISocket() :
		m_socketType(SOCKTYPE_Unknown)
	{
	}

	inline ISocket(SocketType InSocketType) :
		m_socketType(InSocketType)
	{ 
	}

	inline virtual ~ISocket()
	{ 
	}

	virtual bool Close() = 0;
	virtual bool Bind(int16_t port = DEFAULT_SERVER_PORT) = 0;
	virtual bool Connect(const std::string& addrStr) = 0;
	virtual bool Listen() = 0;
	virtual bool WaitForPendingConnection(bool& hasPendingConnection, std::chrono::milliseconds t) = 0;
	virtual bool HasPendingData(uint32_t& pendingDataSize) = 0;
	virtual class ISocket* Accept() = 0;
	virtual bool SendTo(const uint8_t* data, int32_t count, int32_t& sent, const std::string& addrDest);
	virtual bool Send(const uint8_t* data, int32_t count, int32_t& sent);
	virtual bool RecvFrom(uint8_t* data, int32_t size, int32_t& read, std::string& srcAddr, SocketReceiveFlags flags = SocketReceiveFlags::None);
	virtual bool Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags = SocketReceiveFlags::None);
	virtual bool Wait(SocketWaitConditions cond, std::chrono::milliseconds t) = 0;
	virtual SocketConnectionState GetConnectionState() = 0;
	virtual void GetAddress(std::string& outAddr) = 0;
	virtual bool GetPeerAddress(std::string& outAddr) = 0;
	virtual bool SetNonBlocking(bool isNonBlocking = true) = 0;
	virtual bool SetBroadcast(bool allowBroadcast = true) = 0;
	virtual bool JoinMulticastGroup(const std::string& addrStr) = 0;
	virtual bool LeaveMulticastGroup(const std::string& addrStr) = 0;
	virtual bool SetMulticastLoopback(bool loopback) = 0;
	virtual bool SetMulticastTtl(uint8_t timeToLive) = 0;
	virtual bool SetReuseAddr(bool allowReuse = true) = 0;
	virtual bool SetLinger(bool shouldLinger = true, int32_t t = 0) = 0;
	virtual bool SetSendBufferSize(int32_t size, int32_t& newSize) = 0;
	virtual bool SetReceiveBufferSize(int32_t size, int32_t& newSize) = 0;
	virtual int32_t GetPortNo() = 0;

	inline SocketType GetISocketType() const
	{
		return m_socketType;
	}
};