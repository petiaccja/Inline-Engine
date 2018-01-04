#pragma once

#include <string>

#include <BaseLibrary/Serialization/BinarySerializer.hpp>

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

		inline uint8_t *Serialize(int32_t &size)
		{
			BinarySerializer serializer;
			serializer << m_reason.c_str(); // std::string?
			serializer << m_reasonID;
			serializer << m_clientID;
			//return serializer. // get data?
			size = serializer.Size();
			return nullptr;
		}

		static inline DisconnectedEvent Deserialize(const uint8_t *buffer)
		{
			DisconnectedEvent ev;
			BinarySerializer serializer;
			//char *reason = nullptr;
			//reason << serializer;
			ev.m_reasonID << serializer; // is this the correct order?
			ev.m_clientID << serializer;
			return ev;
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