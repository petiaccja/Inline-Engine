#include "NetworkMessage.hpp"

#include "NetworkHeader.hpp"
#include "BitConverter.hpp"

#include <cstring>

namespace inl::net
{
	uint8_t * NetworkMessage::SerializeData(uint32_t & size)
	{
		int32_t sizeOfNetHeader = sizeof(NetworkHeader);

		NetworkHeader header;
		header.Size = 13 + m_dataSize + sizeOfNetHeader;

		uint8_t *bytes = new uint8_t[header.Size];
		memcpy(bytes, &header, sizeOfNetHeader);

		uint8_t *sender = BitConverter::ToBytes<uint32_t>(m_senderID); // 4
		uint8_t *destination = BitConverter::ToBytes<uint32_t>(m_destinationID); // 4
		uint8_t *tag = BitConverter::ToBytes<uint32_t>(m_tag); // 4

		memcpy(bytes + sizeOfNetHeader, sender, 4);
		bytes[sizeOfNetHeader + 4] = (uint8_t)m_distributionMode;
		memcpy(bytes + sizeOfNetHeader + 5, destination, 4);
		memcpy(bytes + sizeOfNetHeader + 9, tag, 4);

		memcpy(bytes + 13 + sizeOfNetHeader, m_data, m_dataSize);

		size = header.Size;
		return bytes;
	}

	void NetworkMessage::Deserialize(uint8_t * data, int32_t size)
	{
		NetworkHeader header;
		int32_t sizeOfNetHeader = sizeof(NetworkHeader);
		memcpy(&header, data, sizeOfNetHeader);

		// int32_t realSize = size - sizeOfNetHeader; // maybe instead of doing this just strip the bytes off the array?
		memcpy(&(m_senderID), data + sizeOfNetHeader, 4);
		m_distributionMode = (DistributionMode)data[sizeOfNetHeader + 4];
		memcpy(&(m_destinationID), data + sizeOfNetHeader + 5, 4);
		memcpy(&(m_tag), data + sizeOfNetHeader + 9, 4);

		m_dataSize = size - sizeOfNetHeader - 13;
		memcpy(m_data, data + sizeOfNetHeader + 13, m_dataSize);
	}
}