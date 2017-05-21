#include "TcpServer.hpp"

namespace inl::net::tcp
{
	bool TcpServer::Bind(int port)
	{
		initialize(port);

		if (bind(soc, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR)
		{
			closesocket(soc);
			return false;
		}

		unsigned long nNoBlock = 1;
		return ioctlsocket(soc, FIONBIO, &nNoBlock) == NO_ERROR;
	}

	bool TcpServer::HasPendingConnections()
	{
		setup_fd_sets();
		return select(0, &read_set, &write_set, &except_set, 0) > 0 ? FD_ISSET(soc, &read_set) : false;
	}

	bool TcpServer::Accept(TcpClient & client)
	{
		sockaddr_in addr;
		socklen_t addr_length = sizeof(addr);
		SOCKET client_ptr = accept(soc, (sockaddr*)(&addr), &addr_length);
		if (client_ptr == INVALID_SOCKET)
			return false;
		
		client = TcpClient(client_ptr);
		client.SetIP(inet_ntop(AF_INET, &(addr.sin_addr), new char[INET_ADDRSTRLEN](), INET_ADDRSTRLEN));
		client.SetPort(ntohs(addr.sin_port));
		return true;
	}

	bool TcpServer::Listen(int max_connections)
	{
		if (listen(soc, max_connections == 0 ? SOMAXCONN : max_connections) != NO_ERROR)
		{
			closesocket(soc);
			return false;
		}
		return true;
	}

	bool TcpServer::initialize(int port)
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

		if (bind(soc, result->ai_addr, (int)result->ai_addrlen) != NO_ERROR)
		{
			freeaddrinfo(result);
			closesocket(soc);
			return false;
		}

		freeaddrinfo(result);
		return (initialized = true);
	}

	void TcpServer::setup_fd_sets()
	{
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_ZERO(&except_set);

		if (soc != INVALID_SOCKET) 
		{
			FD_SET(soc, &read_set);
			FD_SET(soc, &except_set);
		}
	}
}