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
		uint32_t m_senderID;
		DistributionMode m_distributionMode;
		uint32_t m_destinationID;
		uint32_t m_tag;

		void *m_data;

	private:
		uint32_t data_size;
	
	public:
		template<typename T>
		void SetData(T *data)
		{
			m_data = data;
			data_size = sizeof(T);
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

		uint8_t * SerializeData(uint32_t &size)
		{
			int32_t sizeOfNetHeader = sizeof(NetworkHeader);

			NetworkHeader header;
			header.Size = 13 + sizeOfNetHeader + data_size;

			uint8_t *bytes = new uint8_t[header.Size];
			memcpy(bytes, &header, sizeOfNetHeader);

			uint8_t *sender = BitConverter::ToBytes<uint32_t>(m_senderID); // 4
			uint8_t *destination = BitConverter::ToBytes<uint32_t>(m_destinationID); // 4
			uint8_t *tag = BitConverter::ToBytes<uint32_t>(m_tag); // 4

			memcpy(bytes + sizeOfNetHeader, sender, 4);
			bytes[sizeOfNetHeader + 4] = (uint8_t)m_distributionMode;
			memcpy(bytes + sizeOfNetHeader + 5, destination, 4);
			memcpy(bytes + sizeOfNetHeader + 9, tag, 4);

			memcpy(bytes + 13 + sizeOfNetHeader, m_data, data_size);

			size = header.Size;
			return bytes;
		}

		void Deserialize(uint8_t *data, uint32_t size)
		{
			NetworkHeader buffer;
			uint32_t sizeOfNetHeader = sizeof(NetworkHeader);
			memcpy(&(buffer), data, sizeOfNetHeader);

			memcpy(&(m_senderID), data, 4 + sizeOfNetHeader);
			m_distributionMode = (DistributionMode)data[4 + sizeOfNetHeader];
			memcpy(&(m_destinationID), data + 5 + sizeOfNetHeader, 4 + sizeOfNetHeader);
			memcpy(&(m_tag), data + 9 + sizeOfNetHeader, 4 + sizeOfNetHeader);

			m_data = data + 13 + sizeOfNetHeader;
		}

		template<typename T>
		T *GetData()
		{
			return (T*)data;
		}
	};
}