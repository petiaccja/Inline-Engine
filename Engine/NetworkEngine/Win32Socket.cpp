#include "Win32Socket.hpp"

#include "ws2tcpip.h"

#include "Util.hpp"

namespace inl::net
{
	Win32Socket::Win32Socket(enums::Type type) :
		SocketImpl(type)
	{
		if (type == enums::Type::TCP)
		{
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
		}
		else if (type == enums::Type::UDP)
		{

		}
		else if (type == enums::Type::Unknown)
		{
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
		}
	}

	void Win32Socket::Close()
	{
		closesocket(socket_ptr);
	}

	bool Win32Socket::Bind(uint port) // working
	{
		if (socket_type == enums::Type::TCP)
		{
			hints.ai_flags = AI_PASSIVE;
			SetupSocket(std::string(), port);

			if (bind(socket_ptr, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
			{
				closesocket(socket_ptr);
				return false;
			}
			return true;
		}
		else if (socket_type == enums::Type::UDP)
		{
			return false;
		}

		return false;
	}

	bool Win32Socket::Connect(const std::string & ip, uint port)
	{
		SetupSocket(ip, port);

		if (isConnected = connect(socket_ptr, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) 
		{
			closesocket(socket_ptr);
			socket_ptr = INVALID_SOCKET;
		}

		freeaddrinfo(result);
		return isConnected;
	}

	bool Win32Socket::Disconnect()
	{
		if (shutdown(socket_ptr, SD_BOTH) == SOCKET_ERROR) 
		{
			closesocket(socket_ptr);
			isConnected = false;
			return true;
		}
		isConnected = false;
		return false;
	}

	bool Win32Socket::Listen(uint max_conections) // working
	{
		if (listen(socket_ptr, max_conections == 0 ? SOMAXCONN : max_conections) == SOCKET_ERROR)
		{
			closesocket(socket_ptr);
			return false;
		}
		return true;
	}

	bool Win32Socket::HasPendingConnection() // not working
	{
		fd_set sock_set;
		FD_ZERO(&sock_set);
		FD_SET(socket_ptr, &sock_set);

		/*if (select(socket_ptr + 1, nullptr, nullptr, &sock_set, nullptr) == 0 &&
			select(socket_ptr + 1, &sock_set, nullptr, nullptr, nullptr) > 0)
		{
			return true;
		}*/

		return false;
	}

	bool Win32Socket::HasPendingData(uint & size) // working
	{
		if (ioctlsocket(socket_ptr, FIONREAD, (u_long*)(&size)) == 0)
			return size > 0;
		return false;
	}

	SocketImpl * Win32Socket::Accept()
	{
		struct sockaddr *res = (struct sockaddr*)malloc(sizeof(struct sockaddr*));
		SOCKET client_ptr = accept(socket_ptr, res, nullptr);
		if (client_ptr == INVALID_SOCKET)
		{
			closesocket(client_ptr);
			Delete(res);
			return nullptr;
		}

		Win32Socket *ret_sock = new Win32Socket(GetSocketType(), client_ptr);

		if (!ret_sock || !res)
		{
			closesocket(client_ptr);
			Delete(ret_sock);
			Delete(res);
			return nullptr;
		}

		/* // not working very well
		if (GetIPVersion() == enums::IPVersion::IPv4)
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in*)res;
			char ip_address[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(ipv4->sin_addr), ip_address, INET_ADDRSTRLEN);
			ret_sock->IPAddress = std::string(ip_address);
			ret_sock->Port = ipv4->sin_port;
			delete ipv4;
			ipv4 = nullptr;
		}
		else if (GetIPVersion() == enums::IPVersion::IPv6)
		{
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*)res;
			char ip_address[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip_address, INET6_ADDRSTRLEN);
			ret_sock->IPAddress = std::string(ip_address);
			ret_sock->Port = ipv6->sin6_port;
			delete ipv6;
			ipv6 = nullptr;
		}
		*/
		ret_sock->isConnected = true;

		Delete(res);
		return ret_sock;
	}

	bool Win32Socket::SendTo(const char * data, uint count, uint & bytes_sent, const std::string & dest_ip, uint dest_port)
	{
		return false;
	}

	bool Win32Socket::Send(const char * data, uint count, uint & bytes_sent) // working
	{
		return bytes_sent = send(socket_ptr, (const char*)data, count, 0) > 0;
	}

	bool Win32Socket::ReceiveFrom(const char * data, uint size, uint & bytes_read, const std::string & source_ip, uint & source_port, enums::ReceiveFlag flags)
	{
		return false;
	}

	bool Win32Socket::Receive(const char * data, uint size, uint & bytes_read, enums::ReceiveFlag flags) // working, in UE4 theres a method called TranslateFlags????
	{
		bytes_read = recv(socket_ptr, const_cast<char*>(data), size, (int)flags);
		if (bytes_read > 0)
			return true;
		else if (bytes_read == 0)
			return isConnected = false;
		return false;
	}

	bool Win32Socket::Wait(enums::WaitCondition condition, uint wait_time)
	{
		return false;
	}

	enums::ConnectionState Win32Socket::GetConnectionState()
	{
		return enums::ConnectionState();
	}

	bool Win32Socket::GetPeerAddress(std::string & out_ip, uint & out_port)
	{
		return false;
	}

	bool Win32Socket::SetNonBlocking(bool non_blocking)
	{
		return false;
	}

	bool Win32Socket::SetBroadcast(bool allow_broadcast)
	{
		return false;
	}

	bool Win32Socket::JoinMulticastGroup(const std::string & ip, uint port)
	{
		return false;
	}

	bool Win32Socket::LeaveMulticastGroup(const std::string & ip, uint port)
	{
		return false;
	}

	bool Win32Socket::SetMulticastLoopback(bool loopback)
	{
		return false;
	}
	bool Win32Socket::SetMulticastTtl(unsigned short time_to_live)
	{
		return false;
	}

	bool Win32Socket::SetupSocket(const std::string &ip, uint port) // working
	{
		if (ip.size() > 0)
		{
			char *port_buff = new char[sizeof(uint)]();
			_itoa(port, port_buff, 10);
			if (getaddrinfo(ip.c_str(), port_buff, &hints, &result) != 0)
				return false;
		}
		else
		{
			char *port_buff = new char[sizeof(uint)]();
			_itoa(port, port_buff, 10);
			if (getaddrinfo(nullptr, port_buff, &hints, &result) != 0)
				return false;
		}

		if ((socket_ptr = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			return false;
		}

		return true;
	}
}