#include <BaseLibrary/Exception/Exception.hpp>

#include "Socket.hpp"
#include "IPAddress.hpp"

namespace inl::net::sockets
{
	void Socket::init()
	{
		if (GetSocketType() == SocketType::Unknown)
			throw inl::InvalidArgumentException("Unknown socket type");

		if (m_socket == INVALID_SOCKET)
		{
			m_socket = socket(AF_INET, (int)GetSocketType(), 0);

			if (m_socket == INVALID_SOCKET)
				throw inl::RuntimeException("Couldnt create socket");
		}

		if (GetSocketType() == SocketType::Streaming)
		{
			int yes = 1;
			// Disable the Nagle algorithm (i.e. removes buffering of TCP packets)
			setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&yes), sizeof(yes));

			// On Mac OS X, disable the SIGPIPE signal on disconnection
#if defined(__APPLE__) && defined(__MACH__)
			setsockopt(m_socket, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<char*>(&yes), sizeof(yes));
#endif
		}
		else
		{
			// Enable broadcast by default for UDP sockets
			int yes = 1;
			setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&yes), sizeof(yes));
		}
	}

	bool Socket::Close()
	{
		if (m_socket != INVALID_SOCKET)
		{
			int32_t error = closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return error == 0;
		}
		return false;
	}

	bool Socket::Bind(const IPAddress &addr)
	{
		sockaddr_in addr_in = addr.ToCAddr();
		return bind(m_socket, (sockaddr*)&addr_in, sizeof(sockaddr_in)) == 0;
	}

	bool Socket::Connect(const IPAddress& addr)
	{
		sockaddr_in addr_in = addr.ToCAddr();
		int32_t Return = connect(m_socket, (sockaddr*)&addr_in, sizeof(sockaddr_in));
		SocketErrors Error = TranslateErrorCode(Return);

		// "would block" is not an error
		return ((Error == SocketErrors::SE_NO_ERROR) || (Error == SocketErrors::SE_EWOULDBLOCK));
	}

	bool Socket::WaitForPendingConnection(bool& hasPendingConnection, std::chrono::milliseconds t)
	{
		bool hasSucceeded = false;
		hasPendingConnection = false;

		if (HasState(SocketParam::HasError) == SocketReturn::No)
		{
			SocketReturn state = HasState(SocketParam::CanRead, t);

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
			if (ioctl(m_socket, FIONREAD, 
				#if defined(_WIN32) 
					(u_long*) 
				#endif
				&pendingDataSize) == 0)
				return (pendingDataSize > 0);
		}

		return false;
	}

	std::unique_ptr<Socket> Socket::Accept()
	{
		SOCKET newSocket = accept(m_socket, nullptr, nullptr);

		if (newSocket != INVALID_SOCKET)
		{
			return std::make_unique<Socket>(newSocket, GetSocketType());
		}

		return nullptr;
	}

	bool Socket::SendTo(const uint8_t* data, int32_t count, int32_t& sent, const IPAddress& addrDest)
	{
		sockaddr_in addr = addrDest.ToCAddr();
		sent = sendto(m_socket, (const char*)data, count, 0, (sockaddr*)&addr, sizeof(sockaddr_in));

		bool result = sent >= 0;
		if (result)
			m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();

		return result;
	}

	bool Socket::Send(const uint8_t* data, int32_t count, int32_t& sent)
	{
		sent = send(m_socket, (const char*)data, count, 0);

		bool result = sent != SOCKET_ERROR;
		if (result)
			m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();

		return result;
	}

	bool Socket::RecvFrom(uint8_t* data, int32_t size, int32_t& read, IPAddress& srcAddr, SocketReceiveFlags flags)
	{
		socklen_t len = sizeof(sockaddr_in);
		sockaddr_in addr = srcAddr.ToCAddr();
		const int translatedFlags = TranslateFlags(flags);

		read = recvfrom(m_socket, (char*)data, size, translatedFlags, (sockaddr*)&addr, &len);

		if (read < 0 && TranslateErrorCode(read) == SocketErrors::SE_EWOULDBLOCK)
			read = 0;
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

		if (read < 0 && TranslateErrorCode(read) == SocketErrors::SE_EWOULDBLOCK)
			read = 0;
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
				return true;
		}

		if ((cond == SocketWaitConditions::WaitForWrite) || (cond == SocketWaitConditions::WaitForReadOrWrite))
		{
			if (HasState(SocketParam::CanWrite, t) == SocketReturn::Yes)
				return true;
		}

		return false;
	}

	SocketConnectionState Socket::GetConnectionState()
	{
		SocketConnectionState currentState = SocketConnectionState::ConnectionError;

		if (HasState(SocketParam::HasError) == SocketReturn::No)
		{
			if (std::chrono::system_clock::now().time_since_epoch().count() - m_lastActivityTime > std::chrono::milliseconds(5).count())
			{
				SocketReturn writeState = HasState(SocketParam::CanWrite, std::chrono::milliseconds(1));
				SocketReturn readState = HasState(SocketParam::CanRead, std::chrono::milliseconds(1));

				if (writeState == SocketReturn::Yes || readState == SocketReturn::Yes)
				{
					currentState = SocketConnectionState::Connected;
					m_lastActivityTime = std::chrono::system_clock::now().time_since_epoch().count();
				}
				else if (writeState == SocketReturn::No && readState == SocketReturn::No)
					currentState = SocketConnectionState::NotConnected;
			}
			else
				currentState = SocketConnectionState::Connected;
		}

		return currentState;
	}

	void Socket::GetAddress(IPAddress& outAddr)
	{
		struct sockaddr_in addr;
		socklen_t Size = sizeof(sockaddr_in);

		if (getsockname(m_socket, (sockaddr*)&addr, &Size) != 0)
			return;

		outAddr = IPAddress(inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	}

	bool Socket::GetPeerAddress(IPAddress& outAddr)
	{
		struct sockaddr_in addr;
		socklen_t size = sizeof(sockaddr_in);

		int result = getpeername(m_socket, (sockaddr*)&addr, &size);
		if (result != 0)
			return false;

		outAddr = IPAddress(inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

		return result == 0;
	}

	bool Socket::SetNonBlocking(bool isNonBlocking)
	{
#if PLATFORM_HTML5 // if we have more platforms later (html5, android, ios) later we need to do some changes to networking
		throw std::exception("Can't have blocking sockets on HTML5");
		return false;
#else 

#if _WIN32
		return ioctl(m_socket, FIONBIO, (u_long*)&isNonBlocking) == 0;
#else 
		int flags = fcntl(m_socket, F_GETFL, 0);
		flags = isNonBlocking ? flags | O_NONBLOCK : flags ^ (flags & O_NONBLOCK);
		int err = fcntl(m_socket, F_SETFL, flags);
		return (err == 0 ? true : false);
#endif
#endif 
	}

	bool Socket::JoinMulticastGroup(const IPAddress& addrStr)
	{
		sockaddr_in addr = addrStr.ToCAddr();

		ip_mreq imr;
		imr.imr_interface.s_addr = INADDR_ANY;
		imr.imr_multiaddr = addr.sin_addr;

		return (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&imr, sizeof(imr)) == 0);
	}

	bool Socket::LeaveMulticastGroup(const IPAddress& addrStr)
	{
		sockaddr_in addr = addrStr.ToCAddr();

		ip_mreq imr;
		imr.imr_interface.s_addr = INADDR_ANY;
		imr.imr_multiaddr = addr.sin_addr;

		return (setsockopt(m_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&imr, sizeof(imr)) == 0);
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
		bool success = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int32_t)) == 0;

		getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&newSize, &len);

		return success;
	}

	uint32_t Socket::GetPort()
	{
		sockaddr_in addr;
		socklen_t size = sizeof(sockaddr_in);
		if (getsockname(m_socket, (sockaddr*)&addr, &size) != 0)
			return 0; // invalid port
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
		case 0: return SocketErrors::SE_NO_ERROR;
		case EINTR: return SocketErrors::SE_EINTR;
		case EBADF: return SocketErrors::SE_EBADF;
		case EACCES: return SocketErrors::SE_EACCES;
		case EFAULT: return SocketErrors::SE_EFAULT;
		case EINVAL: return SocketErrors::SE_EINVAL;
		case EMFILE: return SocketErrors::SE_EMFILE;
		case EWOULDBLOCK: return SocketErrors::SE_EWOULDBLOCK;
		case EINPROGRESS: return SocketErrors::SE_EINPROGRESS;
		case EALREADY: return SocketErrors::SE_EALREADY;
		case ENOTSOCK: return SocketErrors::SE_ENOTSOCK;
		case EDESTADDRREQ: return SocketErrors::SE_EDESTADDRREQ;
		case EMSGSIZE: return SocketErrors::SE_EMSGSIZE;
		case EPROTOTYPE: return SocketErrors::SE_EPROTOTYPE;
		case ENOPROTOOPT: return SocketErrors::SE_ENOPROTOOPT;
		case EPROTONOSUPPORT: return SocketErrors::SE_EPROTONOSUPPORT;
		case ESOCKTNOSUPPORT: return SocketErrors::SE_ESOCKTNOSUPPORT;
		case EOPNOTSUPP: return SocketErrors::SE_EOPNOTSUPP;
		case EPFNOSUPPORT: return SocketErrors::SE_EPFNOSUPPORT;
		case EAFNOSUPPORT: return SocketErrors::SE_EAFNOSUPPORT;
		case EADDRINUSE: return SocketErrors::SE_EADDRINUSE;
		case EADDRNOTAVAIL: return SocketErrors::SE_EADDRNOTAVAIL;
		case ENETDOWN: return SocketErrors::SE_ENETDOWN;
		case ENETUNREACH: return SocketErrors::SE_ENETUNREACH;
		case ENETRESET: return SocketErrors::SE_ENETRESET;
		case ECONNABORTED: return SocketErrors::SE_ECONNABORTED;
		case ECONNRESET: return SocketErrors::SE_ECONNRESET;
		case ENOBUFS: return SocketErrors::SE_ENOBUFS;
		case EISCONN: return SocketErrors::SE_EISCONN;
		case ENOTCONN: return SocketErrors::SE_ENOTCONN;
		case ESHUTDOWN: return SocketErrors::SE_ESHUTDOWN;
		case ETOOMANYREFS: return SocketErrors::SE_ETOOMANYREFS;
		case ETIMEDOUT: return SocketErrors::SE_ETIMEDOUT;
		case ECONNREFUSED: return SocketErrors::SE_ECONNREFUSED;
		case ELOOP: return SocketErrors::SE_ELOOP;
		case ENAMETOOLONG: return SocketErrors::SE_ENAMETOOLONG;
		case EHOSTDOWN: return SocketErrors::SE_EHOSTDOWN;
		case EHOSTUNREACH: return SocketErrors::SE_EHOSTUNREACH;
		case ENOTEMPTY: return SocketErrors::SE_ENOTEMPTY;
		case EUSERS: return SocketErrors::SE_EUSERS;
		case EDQUOT: return SocketErrors::SE_EDQUOT;
		case ESTALE: return SocketErrors::SE_ESTALE;
		case EREMOTE: return SocketErrors::SE_EREMOTE;
		case ENODEV: return SocketErrors::SE_NODEV;
#if !PLATFORM_HAS_NO_EPROCLIM
		case EPROCLIM: return SocketErrors::SE_EPROCLIM;
#endif
			// 	case EDISCON: return SE_EDISCON;
			// 	case SYSNOTREADY: return SE_SYSNOTREADY;
			// 	case VERNOTSUPPORTED: return SE_VERNOTSUPPORTED;
			// 	case NOTINITIALISED: return SE_NOTINITIALISED;

#if PLATFORM_HAS_BSD_SOCKET_FEATURE_GETHOSTNAME
		case HOST_NOT_FOUND: return SocketErrors::SE_HOST_NOT_FOUND;
		case TRY_AGAIN: return SocketErrors::SE_TRY_AGAIN;
		case NO_RECOVERY: return SocketErrors::SE_NO_RECOVERY;
#endif

			//	case NO_DATA: return SE_NO_DATA;
			// case : return SE_UDP_ERR_PORT_UNREACH; //@TODO Find it's replacement
		}

		return SocketErrors::SE_EINVAL;
#else
		// handle the generic -1 error
		if (code == SOCKET_ERROR)
		{
			return SocketErrors::SE_SOCKET_ERROR;
		}

		switch (code)
		{
		case 0: return SocketErrors::SE_NO_ERROR;
		case ERROR_INVALID_HANDLE: return SocketErrors::SE_ECONNRESET; // invalid socket handle catch
		case WSAEINTR: return SocketErrors::SE_EINTR;
		case WSAEBADF: return SocketErrors::SE_EBADF;
		case WSAEACCES: return SocketErrors::SE_EACCES;
		case WSAEFAULT: return SocketErrors::SE_EFAULT;
		case WSAEINVAL: return SocketErrors::SE_EINVAL;
		case WSAEMFILE: return SocketErrors::SE_EMFILE;
		case WSAEWOULDBLOCK: return SocketErrors::SE_EWOULDBLOCK;
		case WSAEINPROGRESS: return SocketErrors::SE_EINPROGRESS;
		case WSAEALREADY: return SocketErrors::SE_EALREADY;
		case WSAENOTSOCK: return SocketErrors::SE_ENOTSOCK;
		case WSAEDESTADDRREQ: return SocketErrors::SE_EDESTADDRREQ;
		case WSAEMSGSIZE: return SocketErrors::SE_EMSGSIZE;
		case WSAEPROTOTYPE: return SocketErrors::SE_EPROTOTYPE;
		case WSAENOPROTOOPT: return SocketErrors::SE_ENOPROTOOPT;
		case WSAEPROTONOSUPPORT: return SocketErrors::SE_EPROTONOSUPPORT;
		case WSAESOCKTNOSUPPORT: return SocketErrors::SE_ESOCKTNOSUPPORT;
		case WSAEOPNOTSUPP: return SocketErrors::SE_EOPNOTSUPP;
		case WSAEPFNOSUPPORT: return SocketErrors::SE_EPFNOSUPPORT;
		case WSAEAFNOSUPPORT: return SocketErrors::SE_EAFNOSUPPORT;
		case WSAEADDRINUSE: return SocketErrors::SE_EADDRINUSE;
		case WSAEADDRNOTAVAIL: return SocketErrors::SE_EADDRNOTAVAIL;
		case WSAENETDOWN: return SocketErrors::SE_ENETDOWN;
		case WSAENETUNREACH: return SocketErrors::SE_ENETUNREACH;
		case WSAENETRESET: return SocketErrors::SE_ENETRESET;
		case WSAECONNABORTED: return SocketErrors::SE_ECONNABORTED;
		case WSAECONNRESET: return SocketErrors::SE_ECONNRESET;
		case WSAENOBUFS: return SocketErrors::SE_ENOBUFS;
		case WSAEISCONN: return SocketErrors::SE_EISCONN;
		case WSAENOTCONN: return SocketErrors::SE_ENOTCONN;
		case WSAESHUTDOWN: return SocketErrors::SE_ESHUTDOWN;
		case WSAETOOMANYREFS: return SocketErrors::SE_ETOOMANYREFS;
		case WSAETIMEDOUT: return SocketErrors::SE_ETIMEDOUT;
		case WSAECONNREFUSED: return SocketErrors::SE_ECONNREFUSED;
		case WSAELOOP: return SocketErrors::SE_ELOOP;
		case WSAENAMETOOLONG: return SocketErrors::SE_ENAMETOOLONG;
		case WSAEHOSTDOWN: return SocketErrors::SE_EHOSTDOWN;
		case WSAEHOSTUNREACH: return SocketErrors::SE_EHOSTUNREACH;
		case WSAENOTEMPTY: return SocketErrors::SE_ENOTEMPTY;
		case WSAEPROCLIM: return SocketErrors::SE_EPROCLIM;
		case WSAEUSERS: return SocketErrors::SE_EUSERS;
		case WSAEDQUOT: return SocketErrors::SE_EDQUOT;
		case WSAESTALE: return SocketErrors::SE_ESTALE;
		case WSAEREMOTE: return SocketErrors::SE_EREMOTE;
		case WSAEDISCON: return SocketErrors::SE_EDISCON;
		case WSASYSNOTREADY: return SocketErrors::SE_SYSNOTREADY;
		case WSAVERNOTSUPPORTED: return SocketErrors::SE_VERNOTSUPPORTED;
		case WSANOTINITIALISED: return SocketErrors::SE_NOTINITIALISED;
		case WSAHOST_NOT_FOUND: return SocketErrors::SE_HOST_NOT_FOUND;
		case WSATRY_AGAIN: return SocketErrors::SE_TRY_AGAIN;
		case WSANO_RECOVERY: return SocketErrors::SE_NO_RECOVERY;
		case WSANO_DATA: return SocketErrors::SE_NO_DATA;
			// case : return SE_UDP_ERR_PORT_UNREACH;
		}

		return SocketErrors::SE_NO_ERROR;
#endif
	}

	int Socket::TranslateFlags(SocketReceiveFlags flags)
	{
		int translatedFlags = 0;

		if ((int)flags & (int)SocketReceiveFlags::Peek)
		{
			translatedFlags |= MSG_PEEK;
#if !_WIN32
			translatedFlags |= MSG_DONTWAIT;
#endif
		}

		if ((int)flags & (int)SocketReceiveFlags::WaitAll)
		{
			translatedFlags |= MSG_WAITALL;
		}

		return translatedFlags;
	}
}