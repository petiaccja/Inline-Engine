#pragma once

#include "NetworkBuffer.hpp"
#include "Definitions.hpp"

namespace inl::net
{
	class NetworkMessage
	{
	public:
		NetworkMessage() { }
		static NetworkBuffer EncodeMessage(const NetworkMessage &message);
		static NetworkMessage DecodeMessage(const NetworkBuffer &buffer);
		
		int SenderID;
		int DistributionMode;
		int DestinationID;
		int Tag;
		int Subject;
		void *Data;

		bool Valid;
	};
}

// 0 - sender - int
// 0 - tag - 0-255 - byte
// 0 - subject 0-255 - byte
// 0 - data - void*
// 0 - distribution_mode - byte
// 0 - destination_id - if distribution_mode != id -> destination_id = 0 - int
// 
// 
// 
// 
// 
// 
// 
// 