#pragma once

#include "Definitions.hpp"
#include "NetworkBuffer.hpp"
#include "NetworkMessage.hpp"

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

		bool send_net_buffer(const NetworkBuffer &net_buffer);

	private:
		std::string ip;
		int port;

		SOCKET soc;
		struct addrinfo *result;
		struct addrinfo hints;
	};
}