#pragma once

namespace inl::net
{
	enum class DistributionMode
	{
		ID,
		Others,
		All,
		Server
	};

	class NetworkMessage
	{
	public:
		uint32_t SenderID;
		DistributionMode DistributionMode;
		uint16_t Tag;

		void *Data;

		uint8_t *SerializeData(uint32_t &count)
		{

		}

		void Deserialize(uint8_t *data)
		{

		}
	};
}