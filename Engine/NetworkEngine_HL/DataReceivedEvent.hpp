#pragma once

#include "NetworkMessage.hpp"

namespace inl::net::events
{
	class DataReceivedEvent
	{
	public:
		inline DataReceivedEvent(const NetworkMessage &msg)
		{
			SenderID = msg.m_senderID;
			DistributionMode = msg.m_distributionMode;
			DestinationID = msg.m_destinationID;
			Tag = msg.m_tag;
			Data = msg.m_data;
		}

	public:
		uint32_t SenderID;
		DistributionMode DistributionMode;
		uint32_t DestinationID;
		uint32_t Tag;

		void *Data;
	};
}