#include "TcpConnection.hpp"

#include "InternalTags.hpp"

namespace inl::net
{
	TcpConnection::TcpConnection(TcpClient * client) 
		: m_client(client)
	{
	}

	std::shared_ptr<TcpClient> TcpConnection::GetClient()
	{
		return m_client;
	}

	uint32_t TcpConnection::GetID()
	{
		return m_id;
	}

	void TcpConnection::SetID(uint32_t id)
	{
		m_id = id;
	}

	bool TcpConnection::sendMessage(NetworkMessage & msg)
	{
		uint32_t size;
		uint8_t *data = msg.SerializeData(size);
		int32_t sent;
		return m_client->Send(data, size, sent);
	}

	void TcpConnection::ReceiveData()
	{
		std::unique_ptr<uint8_t> header(new uint8_t[sizeof(NetworkHeader*)]());

		int32_t read;
		if (!m_client->Recv(header.get(), sizeof(NetworkHeader*), read))
			return;

		if (read == sizeof(NetworkHeader*))
		{
			std::unique_ptr<NetworkHeader> net_header((NetworkHeader*)header.get());

			std::unique_ptr<uint8_t> buffer(new uint8_t[net_header->Size]());
			int32_t read;
			if (!m_client->Recv(buffer.get(), net_header->Size, read))
			{
				if (read != net_header->Size)
					return; // wrong message?

				NetworkMessage msg;
				msg.Deserialize(buffer.get(), net_header->Size);

				if (msg.GetTag() == (uint32_t)InternalTags::Disconnect)
				{
					//DisconnectedEvent(msg.m_senderID, );
				}
				else if (msg.GetTag() == (uint32_t)InternalTags::Connect)
					NewConnectionEvent(msg.GetSenderID(), msg.GetData<void>());
				else
					DataReceivedEvent(msg.GetSenderID(), msg.GetDistributionMode(), msg.GetDestinationID(), msg.GetTag(), msg.GetData<void>());
			}
		}
		else // wrong message
		{
			return;
		}
	}
}