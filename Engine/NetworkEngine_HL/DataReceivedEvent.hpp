#pragma once

#include "NetworkMessage.hpp"

namespace inl::net::events
{
	class DataReceivedEvent
	{
	public:
		inline DataReceivedEvent(const NetworkMessage &msg)
		{
			SenderID = msg.GetSenderID();
			DistributionMode = msg.GetDistributionMode();
			DestinationID = msg.GetDestinationID();
			Tag = msg.GetTag();
			Data = msg.GetData<void>();
		}

	public:
		uint32_t SenderID;
		DistributionMode DistributionMode;
		uint32_t DestinationID;
		uint32_t Tag;

		void *Data;
	};
}