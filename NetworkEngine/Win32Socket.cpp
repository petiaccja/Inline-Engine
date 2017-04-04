#include "Win32Socket.hpp"

#include "ws2tcpip.h"

namespace inl::net
{
	Win32Socket::Win32Socket(enums::Type type) :
		SocketImpl(type)
	{
		if (type == enums::Type::TCP)
		{
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
		}
		else if (type == enums::Type::UDP)
		{

		}
		else if (type == enums::Type::Unknown)
		{
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
		}
	}

	void Win32Socket::Close()
	{
		closesocket(socket_ptr);
	}

	bool Win32Socket::Bind(uint port)
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

		}
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

	bool Win32Socket::Listen(uint max_conections)
	{
		if (listen(socket_ptr, SOMAXCONN) == SOCKET_ERROR) 
		{
			closesocket(socket_ptr);
			return false;
		}
		return true;
	}

	bool Win32Socket::HasPendingConnection(bool & pending)
	{
		return false;
	}

	bool Win32Socket::HasPendingData(uint & size)
	{
		return false;
	}

	SocketImpl * Win32Socket::Accept()
	{
		
	}

	SocketImpl * Win32Socket::Accept(const std::string & out_addr, uint & out_port)
	{
		SOCKET client_ptr = INVALID_SOCKET;

		struct addrinfo *res;
		int *out_port_ptr;

		client_ptr = accept(client_ptr, NULL, NULL);
		if (client_ptr == INVALID_SOCKET)
		{
			closesocket(client_ptr);
			closesocket(socket_ptr);
			return false;
		}

		struct sockaddr_in *ipv4 = (struct sockaddr_in*)res->ai_addr;
		char ipAddress[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(ipv4->sin_addr), ipAddress, INET_ADDRSTRLEN);
	}

	bool Win32Socket::SendTo(const byte * data, uint count, uint & bytes_sent, const std::string & dest_ip, uint dest_port)
	{
		return false;
	}

	bool Win32Socket::Send(const byte * data, uint count, uint & bytes_sent)
	{
		return bytes_sent = send(socket_ptr, (const char*)data, count, 0) == SOCKET_ERROR;
	}

	bool Win32Socket::ReceiveFrom(const byte * data, uint size, uint & bytes_read, const std::string & source_ip, uint & source_port, enums::ReceiveFlag flags)
	{
		return false;
	}

	bool Win32Socket::Receive(const byte * data, uint size, uint & bytes_read, enums::ReceiveFlag flags)
	{
		int result = recv(socket_ptr, (char*)data, bytes_read, (int)flags);
		if (result > 0)
			return true;
		else if (result == 0)
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

	bool Win32Socket::SetupSocket(const std::string &ip, uint port)
	{
		if (ip.size() > 0)
		{
			if (getaddrinfo(ip.c_str(), (PCSTR)port, &hints, &result) != 0)
				return false;
		}
		else
		{
			if (getaddrinfo(nullptr, (PCSTR)port, &hints, &result) != 0)
				return false;
		}

		if (socket_ptr = socket(result->ai_family, result->ai_socktype, result->ai_protocol) == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			return false;
		}
		freeaddrinfo(result);
		return true;
	}
}