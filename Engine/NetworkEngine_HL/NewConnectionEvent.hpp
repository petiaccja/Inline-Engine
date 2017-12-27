#pragma once

#include <TcpClient.hpp>

namespace inl::net::events
{
	using namespace sockets;

	class NewConnectionEvent // server only
	{
	public:
		inline NewConnectionEvent(TcpClient *client, uint32_t id)
			: m_client(client)
			, m_id(id)
		{
		}

	private:
		TcpClient * m_client;
		uint32_t m_id;
	};
}