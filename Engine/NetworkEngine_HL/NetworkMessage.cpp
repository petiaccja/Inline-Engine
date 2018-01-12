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

	void NetworkMessage::Deserialize(uint8_t * data, uint32_t size)
	{
		NetworkHeader buffer;
		uint32_t sizeOfNetHeader = sizeof(NetworkHeader);
		memcpy(&(buffer), data, sizeOfNetHeader);

		memcpy(&(m_senderID), data, 4 + sizeOfNetHeader);
		m_distributionMode = (DistributionMode)data[4 + sizeOfNetHeader];
		memcpy(&(m_destinationID), data + 5 + sizeOfNetHeader, 4 + sizeOfNetHeader);
		memcpy(&(m_tag), data + 9 + sizeOfNetHeader, 4 + sizeOfNetHeader);

		m_dataSize = size - 13 - sizeOfNetHeader;
		m_data = data + 13 + sizeOfNetHeader;
	}
}