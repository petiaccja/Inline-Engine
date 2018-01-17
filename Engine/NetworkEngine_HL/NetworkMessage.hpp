#pragma once

#include "NetworkHeader.hpp"
#include "BitConverter.hpp"

#include <cstdint>
#include <cstring>

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
		NetworkMessage()
		{
		}

		NetworkMessage(uint32_t sender, DistributionMode mode, uint32_t destinationId, uint32_t tag, void *data, uint32_t dataSize)
			: m_senderID(sender)
			, m_distributionMode(mode)
			, m_destinationID(destinationId)
			, m_tag(tag)
			, m_data(data)
			, m_dataSize(dataSize)
		{
		}

		uint32_t GetSenderID() const;
		DistributionMode GetDistributionMode() const;
		uint32_t GetDestinationID() const;
		uint32_t GetTag() const;

	private:
		uint32_t m_senderID;
		DistributionMode m_distributionMode;
		uint32_t m_destinationID;
		uint32_t m_tag;

		void *m_data;
		uint32_t m_dataSize;
	
	public:
		template<typename T>
		void SetData(T *data)
		{
			m_data = data;
			m_dataSize = sizeof(T);
		}

		template<typename T>
		uint8_t * SerializeData(uint32_t &size)
		{
			int32_t sizeOfNetHeader = sizeof(NetworkHeader);

			NetworkHeader header;
			header.Size = 13 + sizeOfNetHeader + sizeof(T);

			uint8_t *bytes = new uint8_t[header.Size];
			memcpy(bytes, &header, sizeOfNetHeader);

			uint8_t *sender = BitConverter::ToBytes<uint32_t>(m_senderID); // 4
			uint8_t *destination = BitConverter::ToBytes<uint32_t>(m_destinationID); // 4
			uint8_t *tag = BitConverter::ToBytes<uint32_t>(m_tag); // 4

			memcpy(bytes + sizeOfNetHeader, sender, 4);
			bytes[sizeOfNetHeader + 4] = (uint8_t)m_distributionMode;
			memcpy(bytes + sizeOfNetHeader + 5, destination, 4);
			memcpy(bytes + sizeOfNetHeader + 9, tag, 4);

			memcpy(bytes + 13 + sizeOfNetHeader, m_data, sizeof(T));

			size = header.Size;
			return bytes;
		}

		uint8_t *SerializeData(uint32_t &size);
		void Deserialize(uint8_t *data, uint32_t size);

		template<typename T>
		T *GetData() const
		{
			return (T*)m_data;
		}
	};
}