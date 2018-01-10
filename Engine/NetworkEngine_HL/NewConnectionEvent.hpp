#pragma once

#include <TcpClient.hpp>

namespace inl::net::events
{
	using namespace sockets;

	class NewConnectionEvent
	{
	public:
		inline NewConnectionEvent(uint32_t id, uint8_t *data)
			: m_id(id)
			, m_data(data)
		{
		}

	private:
		uint32_t m_id;
		uint8_t *m_data;
	};
}