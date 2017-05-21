#pragma once

#include "Definitions.hpp"
#include "NetworkBuffer.hpp"
#include "NetworkMessage.hpp"

#include <string>

namespace inl::net::tcp
{
	class TcpServer
	{
	public:
		bool IsActive();
		void Stop();

	private:
		bool initialize(int port = DEFAULT_LISTEN_PORT);

	private:
		int port;

		bool initialized;

		bool stopping;

		SOCKET soc;
		struct addrinfo *result;
		struct addrinfo hints;
	};
}