#include "TcpServer.hpp"

#ifndef _MSC_VER
	#include <string.h>
#endif

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

	if (getaddrinfo(0, util::IntToStr(port), &hints, &result) != 0)
	{
		return false;
	}

	if ((soc = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		return false;
	}

	if (bind(soc, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		closesocket(soc);
		return false;
	}

	freeaddrinfo(result);
	return (initialized = true);
}
