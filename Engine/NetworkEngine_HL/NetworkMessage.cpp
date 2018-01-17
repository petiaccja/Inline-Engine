#include "NetworkMessage.hpp"

namespace inl::net
{
	uint32_t NetworkMessage::GetSenderID() const
	{
		return m_senderID;
	}

	DistributionMode NetworkMessage::GetDistributionMode() const
	{
		return m_distributionMode;
	}

	uint32_t NetworkMessage::GetDestinationID() const
	{
		return m_destinationID;
	}

	uint32_t NetworkMessage::GetTag() const
	{
		return m_tag;
	}

	uint8_t *NetworkMessage::SerializeData(uint32_t &size)
	{
		int32_t sizeOfNetHeader = sizeof(NetworkHeader);

		NetworkHeader header;
		header.Size = 13 + sizeOfNetHeader + m_dataSize;

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

	void NetworkMessage::Deserialize(uint8_t *data, uint32_t size)
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
}