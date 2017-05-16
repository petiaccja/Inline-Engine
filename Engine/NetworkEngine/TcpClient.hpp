#pragma once

#include "Definitions.hpp"
#include "NetworkBuffer.hpp"

#include <string>

namespace inl::net::tcp
{
	class TcpClient
	{
	public:
		TcpClient(const SOCKET &socket);
		TcpClient(const std::string &ip, int port);
		~TcpClient();

		bool DataAvailable(int &size);

	private:
		bool initialize(const std::string &ip, int port);

		NetworkBuffer receive_buffer();

	private:
		std::string ip;
		int port;

		SOCKET soc = INVALID_SOCKET;
		struct addrinfo *result = nullptr;
		struct addrinfo hints;
	};
}