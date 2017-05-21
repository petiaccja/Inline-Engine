#include "TcpServer.hpp"

bool inl::net::tcp::TcpServer::IsActive()
{
	return (soc != INVALID_SOCKET && !stopping);
}

void inl::net::tcp::TcpServer::Stop()
{
	stopping = true;
}

bool inl::net::tcp::TcpServer::initialize(int port)
{
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(0, itoa(port, new char[0](), 10), &hints, &result) != 0)
	{
		WSACleanup();
		return false;
	}

	if ((soc = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	if (bind(soc, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		closesocket(soc);
		WSACleanup();
		return false;
	}

	freeaddrinfo(result);
	return (initialized = true);
}
