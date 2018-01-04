#pragma once

#include <TcpClient.hpp>

namespace inl::net::events
{
	using namespace sockets;

	class NewConnectionEvent // server only
	{
	public:
		inline NewConnectionEvent(std::shared_ptr<TcpClient> client, uint32_t id)
			: m_client(client)
			, m_id(id)
		{
		}

	private:
		std::shared_ptr<TcpClient> m_client;
		uint32_t m_id;
	};
}