#pragma once

namespace inl::net::enums
{
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

	enum class SocketErrors
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

	enum class SocketType
	{
		Unknown = -1,
		Datagram = 2,
		Streaming = 1,
	};

	enum class SocketProtocol
	{
		IPv4 = 2, // AF_INET
		IPv6 = 23 // AF_INET6
	};

	enum class SocketReceiveFlags
	{
		None = 0,
		Peek = 2,
		WaitAll = 0x100,
	};

	enum class SocketWaitConditions
	{
		WaitForRead,
		WaitForWrite,
		WaitForReadOrWrite,
	};

	enum class SocketConnectionState
	{
		NotConnected,
		Connected,
		ConnectionError,
	};
}