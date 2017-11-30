#include "Socket.hpp"
#include "Util.hpp"

bool Socket::Close(void)
{
	if (m_socket != INVALID_SOCKET)
	{
		int32_t error = closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return error == 0;
	}
	return false;
}


bool Socket::Bind(int16_t port)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	return bind(m_socket, (sockaddr*)&addr, sizeof(sockaddr_in)) == 0;
}


bool Socket::Connect(const std::string& addrStr)
{
	std::vector<std::string> splitted = inl::net::util::Split(addrStr, ":");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(splitted[0].c_str());
	addr.sin_port = htons(atoi(splitted[1].c_str()));

	int32_t Return = connect(m_socket, (sockaddr*)&addr, sizeof(sockaddr_in));

	SocketErrors Error = TranslateErrorCode(Return);

	// "would block" is not an error
	return ((Error == SE_NO_ERROR) || (Error == SE_EWOULDBLOCK));
}


bool Socket::Listen()
{
	return listen(m_socket, SOMAXCONN) == 0;
}


bool Socket::HasPendingConnection(bool& hasPendingConnection)
{
	bool hasSucceeded = false;
	hasPendingConnection = false;

	if (HasState(SocketParam::HasError) == SocketReturn::No)
	{
		SocketReturn state = HasState(SocketParam::CanRead);

		hasSucceeded = state != SocketReturn::EncounteredError;
		hasPendingConnection = state == SocketReturn::Yes;
	}

	return hasSucceeded;
}


bool Socket::HasPendingData(uint32_t& pendingDataSize)
{
	pendingDataSize = 0;

	if (HasState(SocketParam::CanRead) == SocketReturn::Yes)
	{
		if (ioctlsocket(m_socket, FIONREAD, (u_long*)(&pendingDataSize)) == 0)
		{
			return (pendingDataSize > 0);
		}
	}

	return false;
}


ISocket* Socket::Accept()
{
	SOCKET newSocket = accept(m_socket, nullptr, nullptr);

	if (newSocket != INVALID_SOCKET)
	{
		return new Socket(newSocket, m_socketType);
	}

	return nullptr;
}


ISocket* Socket::Accept(std::string& outAddr)
{
	socklen_t SizeOf = sizeof(sockaddr_in);
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	SOCKET NewSocket = accept(m_socket, (sockaddr*)&addr, &SizeOf);
	outAddr = inet_ntoa(addr.sin_addr);
	outAddr += ntohs(addr.sin_port);

	if (NewSocket != INVALID_SOCKET)
	{
		return new Socket(NewSocket, m_socketType);
	}

	return nullptr;
}


bool Socket::SendTo(const uint8_t* data, int32_t count, int32_t& sent, const std::string& addrDest)
{
	std::vector<std::string> splitted = inl::net::util::Split(addrDest, ":");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(splitted[0].c_str());
	addr.sin_port = htons(atoi(splitted[1].c_str()));
	sent = sendto(m_socket, (const char*)data, count, 0, (sockaddr*)&addr, sizeof(sockaddr_in));

	bool result = sent >= 0;
	if (result)
	{
		m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();
	}
	return result;
}


bool Socket::Send(const uint8_t* data, int32_t count, int32_t& sent)
{
	sent = send(m_socket, (const char*)data, count, 0);

	bool result = sent >= 0;
	if (result)
	{
		m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();
	}
	return result;
}


bool Socket::RecvFrom(uint8_t* data, int32_t size, int32_t& read, std::string& srcAddr, SocketReceiveFlags flags)
{
	socklen_t len = sizeof(sockaddr_in);

	std::vector<std::string> splitted = inl::net::util::Split(srcAddr, ":");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(splitted[0].c_str());
	addr.sin_port = htons(atoi(splitted[1].c_str()));

	const int translatedFlags = TranslateFlags(flags);

	read = recvfrom(m_socket, (char*)data, size, translatedFlags, (sockaddr*)&addr, &len);

	if (read < 0 && TranslateErrorCode(read) == SE_EWOULDBLOCK)
	{
		read = 0;
	}
	else if (read <= 0) // 0 means gracefully closed
	{
		read = 0;
		return false;
	}

	m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();

	return true;
}


bool Socket::Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags)
{
	const int translatedFlags = TranslateFlags(flags);
	read = recv(m_socket, (char*)data, size, translatedFlags);

	if (read < 0 && TranslateErrorCode(read) == SE_EWOULDBLOCK)
	{
		read = 0;
	}
	else if (read <= 0) // 0 means gracefully closed
	{
		read = 0;
		return false;
	}

	m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();

	return true;
}


bool Socket::Wait(SocketWaitConditions cond, std::chrono::milliseconds t)
{
	if ((cond == SocketWaitConditions::WaitForRead) || (cond == SocketWaitConditions::WaitForReadOrWrite))
	{
		if (HasState(SocketParam::CanRead, t) == SocketReturn::Yes)
		{
			return true;
		}
	}

	if ((cond == SocketWaitConditions::WaitForWrite) || (cond == SocketWaitConditions::WaitForReadOrWrite))
	{
		if (HasState(SocketParam::CanWrite, t) == SocketReturn::Yes)
		{
			return true;
		}
	}

	return false;
}


SocketConnectionState Socket::GetConnectionState()
{
	SocketConnectionState currentState = SCS_ConnectionError;

	if (HasState(SocketParam::HasError) == SocketReturn::No)
	{
		if (std::chrono::system_clock::now().time_since_epoch().count() - m_lastActivityTime > std::chrono::milliseconds(5).count())
		{
			SocketReturn writeState = HasState(SocketParam::CanWrite, std::chrono::milliseconds(1));
			SocketReturn readState = HasState(SocketParam::CanRead, std::chrono::milliseconds(1));

			if (writeState == SocketReturn::Yes || readState == SocketReturn::Yes)
			{
				currentState = SCS_Connected;
				m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();
			}
			else if (writeState == SocketReturn::No && readState == SocketReturn::No)
			{
				currentState = SCS_NotConnected;
			}
		}
		else
		{
			currentState = SCS_Connected;
		}
	}

	return currentState;
}


void Socket::GetAddress(std::string& outAddr)
{
	struct sockaddr_in addr;
	socklen_t Size = sizeof(sockaddr_in);

	bool success = getsockname(m_socket, (sockaddr*)&addr, &Size) == 0;

	if (!success)
	{
		return;
	}

	outAddr = inet_ntoa(addr.sin_addr);
	outAddr += ntohs(addr.sin_port);
}


bool Socket::GetPeerAddress(std::string& outAddr)
{
	struct sockaddr_in addr;
	socklen_t size = sizeof(sockaddr_in);

	int result = getpeername(m_socket, (sockaddr*)&addr, &size);

	if (result != 0)
	{
		return false;
	}

	outAddr = inet_ntoa(addr.sin_addr);
	outAddr += ntohs(addr.sin_port);

	return result == 0;
}

bool Socket::SetNonBlocking(bool isNonBlocking)
{
#if PLATFORM_HTML5
	ensureMsgf(isNonBlocking, TEXT("Can't have blocking sockets on HTML5"));
	return true;
#else 

#if _WIN32
	u_long value = isNonBlocking ? true : false;
	return ioctlsocket(m_socket, FIONBIO, &value) == 0;
#else 
	int flags = fcntl(m_socket, F_GETFL, 0);
	flags = isNonBlocking ? flags | O_NONBLOCK : flags ^ (flags & O_NONBLOCK);
	int err = fcntl(m_socket, F_SETFL, flags);
	return (err == 0 ? true : false);
#endif
#endif 
}


bool Socket::SetBroadcast(bool allowBroadcast)
{
	int param = allowBroadcast ? 1 : 0;
	return setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (char*)&param, sizeof(param)) == 0;
}


bool Socket::JoinMulticastGroup(const std::string& addrStr)
{
	ip_mreq imr;

	std::vector<std::string> splitted = inl::net::util::Split(addrStr, ":");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(splitted[0].c_str());
	addr.sin_port = htons(atoi(splitted[1].c_str()));

	imr.imr_interface.s_addr = INADDR_ANY;
	imr.imr_multiaddr = addr.sin_addr;

	return (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&imr, sizeof(imr)) == 0);
}


bool Socket::LeaveMulticastGroup(const std::string& addrStr)
{
	ip_mreq imr;

	std::vector<std::string> splitted = inl::net::util::Split(addrStr, ":");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(splitted[0].c_str());
	addr.sin_port = htons(atoi(splitted[1].c_str()));

	imr.imr_interface.s_addr = INADDR_ANY;
	imr.imr_multiaddr = addr.sin_addr;

	return (setsockopt(m_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&imr, sizeof(imr)) == 0);
}


bool Socket::SetMulticastLoopback(bool loopback)
{
	return (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopback, sizeof(loopback)) == 0);
}


bool Socket::SetMulticastTtl(uint8_t timeToLive)
{
	return (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&timeToLive, sizeof(timeToLive)) == 0);
}


bool Socket::SetReuseAddr(bool allowReuse)
{
	int param = allowReuse ? 1 : 0;
	return setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&param, sizeof(param)) == 0;
}


bool Socket::SetLinger(bool shouldLinger, int32_t t)
{
	linger ling;

	ling.l_onoff = shouldLinger;
	ling.l_linger = t;

	return setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling)) == 0;
}


bool Socket::SetSendBufferSize(int32_t size, int32_t& newSize)
{
	socklen_t len = sizeof(int32_t);
	bool success = setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(int32_t)) == 0;

	getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&newSize, &len);

	return success;
}


bool Socket::SetReceiveBufferSize(int32_t size, int32_t& newSize)
{
	socklen_t len = sizeof(int32_t);
	bool bOk = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int32_t)) == 0;

	getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&newSize, &len);

	return bOk;
}


int32_t Socket::GetPortNo()
{
	sockaddr_in addr;

	socklen_t size = sizeof(sockaddr_in);

	bool success = getsockname(m_socket, (sockaddr*)&addr, &size) == 0;

	if (!success)
	{
		return -1;
	}

	return ntohs(addr.sin_port);
}

SocketReturn Socket::HasState(SocketParam state, std::chrono::milliseconds t)
{
	timeval time;
	time.tv_sec = t.count();
	time.tv_usec = t.count() * 1000 + t.count();

	fd_set socketSet;

	FD_ZERO(&socketSet);
	FD_SET(m_socket, &socketSet);

	int32_t SelectStatus = 0;
	switch (state)
	{
	case SocketParam::CanRead:
		SelectStatus = select(m_socket + 1, &socketSet, nullptr, nullptr, &time);
		break;

	case SocketParam::CanWrite:
		SelectStatus = select(m_socket + 1, nullptr, &socketSet, nullptr, &time);
		break;

	case SocketParam::HasError:
		SelectStatus = select(m_socket + 1, nullptr, nullptr, &socketSet, &time);
		break;
	}

	return SelectStatus > 0 ? SocketReturn::Yes :
		SelectStatus == 0 ? SocketReturn::No :
		SocketReturn::EncounteredError;
}

SocketErrors Socket::TranslateErrorCode(int32_t code)
{
#if !_WIN32
	if (code == SOCKET_ERROR)
	{
		return SE_SOCKET_ERROR;
	}

	switch (code)
	{
	case 0: return SE_NO_ERROR;
	case EINTR: return SE_EINTR;
	case EBADF: return SE_EBADF;
	case EACCES: return SE_EACCES;
	case EFAULT: return SE_EFAULT;
	case EINVAL: return SE_EINVAL;
	case EMFILE: return SE_EMFILE;
	case EWOULDBLOCK: return SE_EWOULDBLOCK;
	case EINPROGRESS: return SE_EINPROGRESS;
	case EALREADY: return SE_EALREADY;
	case ENOTSOCK: return SE_ENOTSOCK;
	case EDESTADDRREQ: return SE_EDESTADDRREQ;
	case EMSGSIZE: return SE_EMSGSIZE;
	case EPROTOTYPE: return SE_EPROTOTYPE;
	case ENOPROTOOPT: return SE_ENOPROTOOPT;
	case EPROTONOSUPPORT: return SE_EPROTONOSUPPORT;
	case ESOCKTNOSUPPORT: return SE_ESOCKTNOSUPPORT;
	case EOPNOTSUPP: return SE_EOPNOTSUPP;
	case EPFNOSUPPORT: return SE_EPFNOSUPPORT;
	case EAFNOSUPPORT: return SE_EAFNOSUPPORT;
	case EADDRINUSE: return SE_EADDRINUSE;
	case EADDRNOTAVAIL: return SE_EADDRNOTAVAIL;
	case ENETDOWN: return SE_ENETDOWN;
	case ENETUNREACH: return SE_ENETUNREACH;
	case ENETRESET: return SE_ENETRESET;
	case ECONNABORTED: return SE_ECONNABORTED;
	case ECONNRESET: return SE_ECONNRESET;
	case ENOBUFS: return SE_ENOBUFS;
	case EISCONN: return SE_EISCONN;
	case ENOTCONN: return SE_ENOTCONN;
	case ESHUTDOWN: return SE_ESHUTDOWN;
	case ETOOMANYREFS: return SE_ETOOMANYREFS;
	case ETIMEDOUT: return SE_ETIMEDOUT;
	case ECONNREFUSED: return SE_ECONNREFUSED;
	case ELOOP: return SE_ELOOP;
	case ENAMETOOLONG: return SE_ENAMETOOLONG;
	case EHOSTDOWN: return SE_EHOSTDOWN;
	case EHOSTUNREACH: return SE_EHOSTUNREACH;
	case ENOTEMPTY: return SE_ENOTEMPTY;
	case EUSERS: return SE_EUSERS;
	case EDQUOT: return SE_EDQUOT;
	case ESTALE: return SE_ESTALE;
	case EREMOTE: return SE_EREMOTE;
	case ENODEV: return SE_NODEV;
#if !PLATFORM_HAS_NO_EPROCLIM
	case EPROCLIM: return SE_EPROCLIM;
#endif
		// 	case EDISCON: return SE_EDISCON;
		// 	case SYSNOTREADY: return SE_SYSNOTREADY;
		// 	case VERNOTSUPPORTED: return SE_VERNOTSUPPORTED;
		// 	case NOTINITIALISED: return SE_NOTINITIALISED;

#if PLATFORM_HAS_BSD_SOCKET_FEATURE_GETHOSTNAME
	case HOST_NOT_FOUND: return SE_HOST_NOT_FOUND;
	case TRY_AGAIN: return SE_TRY_AGAIN;
	case NO_RECOVERY: return SE_NO_RECOVERY;
#endif

		//	case NO_DATA: return SE_NO_DATA;
		// case : return SE_UDP_ERR_PORT_UNREACH; //@TODO Find it's replacement
	}

	return SE_EINVAL;
#else
	// handle the generic -1 error
	if (code == SOCKET_ERROR)
	{
		return SE_SOCKET_ERROR;
	}

	switch (code)
	{
	case 0: return SE_NO_ERROR;
	case ERROR_INVALID_HANDLE: return SE_ECONNRESET; // invalid socket handle catch
	case WSAEINTR: return SE_EINTR;
	case WSAEBADF: return SE_EBADF;
	case WSAEACCES: return SE_EACCES;
	case WSAEFAULT: return SE_EFAULT;
	case WSAEINVAL: return SE_EINVAL;
	case WSAEMFILE: return SE_EMFILE;
	case WSAEWOULDBLOCK: return SE_EWOULDBLOCK;
	case WSAEINPROGRESS: return SE_EINPROGRESS;
	case WSAEALREADY: return SE_EALREADY;
	case WSAENOTSOCK: return SE_ENOTSOCK;
	case WSAEDESTADDRREQ: return SE_EDESTADDRREQ;
	case WSAEMSGSIZE: return SE_EMSGSIZE;
	case WSAEPROTOTYPE: return SE_EPROTOTYPE;
	case WSAENOPROTOOPT: return SE_ENOPROTOOPT;
	case WSAEPROTONOSUPPORT: return SE_EPROTONOSUPPORT;
	case WSAESOCKTNOSUPPORT: return SE_ESOCKTNOSUPPORT;
	case WSAEOPNOTSUPP: return SE_EOPNOTSUPP;
	case WSAEPFNOSUPPORT: return SE_EPFNOSUPPORT;
	case WSAEAFNOSUPPORT: return SE_EAFNOSUPPORT;
	case WSAEADDRINUSE: return SE_EADDRINUSE;
	case WSAEADDRNOTAVAIL: return SE_EADDRNOTAVAIL;
	case WSAENETDOWN: return SE_ENETDOWN;
	case WSAENETUNREACH: return SE_ENETUNREACH;
	case WSAENETRESET: return SE_ENETRESET;
	case WSAECONNABORTED: return SE_ECONNABORTED;
	case WSAECONNRESET: return SE_ECONNRESET;
	case WSAENOBUFS: return SE_ENOBUFS;
	case WSAEISCONN: return SE_EISCONN;
	case WSAENOTCONN: return SE_ENOTCONN;
	case WSAESHUTDOWN: return SE_ESHUTDOWN;
	case WSAETOOMANYREFS: return SE_ETOOMANYREFS;
	case WSAETIMEDOUT: return SE_ETIMEDOUT;
	case WSAECONNREFUSED: return SE_ECONNREFUSED;
	case WSAELOOP: return SE_ELOOP;
	case WSAENAMETOOLONG: return SE_ENAMETOOLONG;
	case WSAEHOSTDOWN: return SE_EHOSTDOWN;
	case WSAEHOSTUNREACH: return SE_EHOSTUNREACH;
	case WSAENOTEMPTY: return SE_ENOTEMPTY;
	case WSAEPROCLIM: return SE_EPROCLIM;
	case WSAEUSERS: return SE_EUSERS;
	case WSAEDQUOT: return SE_EDQUOT;
	case WSAESTALE: return SE_ESTALE;
	case WSAEREMOTE: return SE_EREMOTE;
	case WSAEDISCON: return SE_EDISCON;
	case WSASYSNOTREADY: return SE_SYSNOTREADY;
	case WSAVERNOTSUPPORTED: return SE_VERNOTSUPPORTED;
	case WSANOTINITIALISED: return SE_NOTINITIALISED;
	case WSAHOST_NOT_FOUND: return SE_HOST_NOT_FOUND;
	case WSATRY_AGAIN: return SE_TRY_AGAIN;
	case WSANO_RECOVERY: return SE_NO_RECOVERY;
	case WSANO_DATA: return SE_NO_DATA;
		// case : return SE_UDP_ERR_PORT_UNREACH;
	}

	return SE_NO_ERROR;
#endif
}

int Socket::TranslateFlags(SocketReceiveFlags flags)
{
	int translatedFlags = 0;

	if (flags & SocketReceiveFlags::Peek)
	{
		translatedFlags |= MSG_PEEK;
#if !_WIN32
		translatedFlags |= MSG_DONTWAIT;
#endif
	}

	if (flags & SocketReceiveFlags::WaitAll)
	{
		translatedFlags |= MSG_WAITALL;
	}

	return translatedFlags;
}
