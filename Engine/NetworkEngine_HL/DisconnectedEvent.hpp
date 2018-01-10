#pragma once

#include <string>

namespace inl::net::events
{
	class DisconnectedEvent // can be processed by client and server
	{
	private:
		DisconnectedEvent();

	public:
		inline DisconnectedEvent(uint32_t id, char *reason, int32_t reason_id)
			: m_reason(reason)
			, m_clientID(id)
			, m_reasonID(reason_id)
		{
		}

		inline DisconnectedEvent(uint32_t id, const std::string &reason, int32_t reason_id)
			: m_reason(reason)
			, m_clientID(id)
			, m_reasonID(reason_id)
		{
		}

		inline const std::string &Reason() const { return m_reason; }
		inline int32_t ReasonID() const { return m_reasonID; }
		inline uint32_t ID() const { return m_clientID; }

	private:
		std::string m_reason;
		int32_t m_reasonID;
		uint32_t m_clientID;
	};
}