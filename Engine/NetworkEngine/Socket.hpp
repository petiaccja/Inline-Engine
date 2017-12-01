#pragma once

#include "ISocket.hpp"

enum class SocketParam
{
	CanRead,
	CanWrite,
	HasError,
};


enum class SocketReturn
{
	Yes,
	No,
	EncounteredError,
};

enum SocketErrors
{
	SE_SOCKET_ERROR = -1,
	SE_NO_ERROR,
	SE_EINTR,
	SE_EBADF,
	SE_EACCES,
	SE_EFAULT,
	SE_EINVAL,
	SE_EMFILE,
	SE_EWOULDBLOCK,
	SE_EINPROGRESS,
	SE_EALREADY,
	SE_ENOTSOCK,
	SE_EDESTADDRREQ,
	SE_EMSGSIZE,
	SE_EPROTOTYPE,
	SE_ENOPROTOOPT,
	SE_EPROTONOSUPPORT,
	SE_ESOCKTNOSUPPORT,
	SE_EOPNOTSUPP,
	SE_EPFNOSUPPORT,
	SE_EAFNOSUPPORT,
	SE_EADDRINUSE,
	SE_EADDRNOTAVAIL,
	SE_ENETDOWN,
	SE_ENETUNREACH,
	SE_ENETRESET,
	SE_ECONNABORTED,
	SE_ECONNRESET,
	SE_ENOBUFS,
	SE_EISCONN,
	SE_ENOTCONN,
	SE_ESHUTDOWN,
	SE_ETOOMANYREFS,
	SE_ETIMEDOUT,
	SE_ECONNREFUSED,
	SE_ELOOP,
	SE_ENAMETOOLONG,
	SE_EHOSTDOWN,
	SE_EHOSTUNREACH,
	SE_ENOTEMPTY,
	SE_EPROCLIM,
	SE_EUSERS,
	SE_EDQUOT,
	SE_ESTALE,
	SE_EREMOTE,
	SE_EDISCON,
	SE_SYSNOTREADY,
	SE_VERNOTSUPPORTED,
	SE_NOTINITIALISED,
	SE_HOST_NOT_FOUND,
	SE_TRY_AGAIN,
	SE_NO_RECOVERY,
	SE_NO_DATA,
	SE_UDP_ERR_PORT_UNREACH,
	SE_ADDRFAMILY,
	SE_SYSTEM,
	SE_NODEV,
	SE_GET_LAST_ERROR_CODE,
};

class Socket : public ISocket
{
public:
	inline Socket(SocketType socketType) :
		ISocket(socketType)
	{
	}

	inline Socket(SOCKET newSocket, SocketType socketType) :
		ISocket(socketType),
		m_socket(newSocket)
	{
	}

	virtual ~Socket()
	{
		Close();
	}

public:
	SOCKET GetNativeSocket()
	{
		return m_socket;
	}

public:

	// ISocket overrides

	virtual bool Close() override;
	virtual bool Bind(int16_t port = DEFAULT_SERVER_PORT) override;
	virtual bool Connect(const std::string& addrStr) override;
	virtual bool Listen() override;
	virtual bool WaitForPendingConnection(bool& hasPendingConnection, std::chrono::milliseconds t) override;
	virtual bool HasPendingData(uint32_t& pendingDataSize) override;
	virtual class ISocket* Accept() override;
	virtual bool SendTo(const uint8_t* data, int32_t count, int32_t& sent, const std::string& addrDest);
	virtual bool Send(const uint8_t* data, int32_t count, int32_t& sent);
	virtual bool RecvFrom(uint8_t* data, int32_t size, int32_t& read, std::string& srcAddr, SocketReceiveFlags flags = SocketReceiveFlags::None);
	virtual bool Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags = SocketReceiveFlags::None);
	virtual bool Wait(SocketWaitConditions cond, std::chrono::milliseconds t) override;
	virtual SocketConnectionState GetConnectionState() override;
	virtual void GetAddress(std::string& outAddr) override;
	virtual bool GetPeerAddress(std::string& outAddr) override;
	virtual bool SetNonBlocking(bool isNonBlocking = true) override;
	virtual bool SetBroadcast(bool allowBroadcast = true) override;
	virtual bool JoinMulticastGroup(const std::string& addrStr) override;
	virtual bool LeaveMulticastGroup(const std::string& addrStr) override;
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
