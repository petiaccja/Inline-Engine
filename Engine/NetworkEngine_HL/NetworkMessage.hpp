#pragma once

#include <cstdint>

namespace inl::net
{
	enum class DistributionMode : uint8_t
	{
		ID,
		Others,
		OthersAndServer,
		All,
		AllAndMe,
		Server
	};

	class NetworkMessage
	{
	public:
		uint32_t m_senderID;
		DistributionMode m_distributionMode;
		uint32_t m_destinationID;
		uint32_t m_tag;

		void *m_data;
		uint32_t m_dataSize;
	
	public:
		uint8_t * SerializeData(uint32_t &size);
		void Deserialize(uint8_t *data, uint32_t size);

		virtual void HandleData() = 0; // this allows custom messages
	};
}