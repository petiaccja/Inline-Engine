#pragma once

#include "Definitions.hpp"
#include "NetworkBuffer.hpp"
#include "NetworkMessage.hpp"
#include "TcpClient.hpp"

#include <string>

namespace inl::net::tcp
{
	class TcpServer
	{
	public:
		inline bool IsActive() { return (soc != INVALID_SOCKET && !stopping); }
		inline void Stop() { stopping = true; }
		bool Bind(int port = DEFAULT_LISTEN_PORT);
		bool HasPendingConnections();
		bool Accept(TcpClient &client);
		bool Listen(int max_connections);

	public:

	private:
		bool initialize(int port);
		void setup_fd_sets();

	private:
		int port;

		bool initialized;

		bool stopping;

		SOCKET soc;
		struct addrinfo *result;
		struct addrinfo hints;

		fd_set read_set;
		fd_set write_set;
		fd_set except_set;
	};
}