#include "TcpConnectionHandler.hpp"

#include "DisconnectedEvent.hpp"
#include "NewConnectionEvent.hpp"
#include "InternalTags.hpp"

#include "TcpConnection.hpp"
#include "NetworkHeader.hpp"

#include <chrono>

namespace inl::net::servers
{
	using namespace events;

	TcpConnectionHandler::TcpConnectionHandler()
		: m_run(true)
	{
		std::thread receive_thread(&TcpConnectionHandler::HandleReceiveThreaded, this);
		m_receiveThread.swap(receive_thread);

		std::thread send_thread(&TcpConnectionHandler::HandleSendThreaded, this);
		m_sendThread.swap(send_thread);
	}

	void TcpConnectionHandler::Add(std::shared_ptr<TcpConnection> &c)
	{
		uint32_t id = GetAvailableID();
		if (id == -1)
		{
			// this can be handled just by the server
			// what if the server owner wants to know if a user wanted to join but couldnt
			DisconnectedEvent disconnected_event(id, "Server Full", -1);
			std::shared_ptr<TcpClient> client = c->GetClient();
			int32_t size = 0;
			uint8_t *buffer = disconnected_event.Serialize(size);
			int32_t sent = 0;
			client->Send(buffer, size, sent);
			client->Close();
		}

		c->SetID(id);

		m_listMutex.lock();
		m_list.push_back(c);
		m_listMutex.unlock();

		uint32_t data_size;
		uint8_t *data;
		if (c->GetClient()->HasPendingData(data_size))
		{
			int32_t read;
			if (c->GetClient()->Recv(data, data_size, read))
			{
				// failed to read
			}
		}

		NetworkMessage msg;
		msg.m_distributionMode = DistributionMode::ID;
		msg.m_destinationID = id;
		msg.m_tag = (uint32_t)InternalTags::AssignID;
		//msg.m_data = id; // id to uint8_t*
		msg.m_dataSize = 4;

		uint32_t serialized_size;
		uint8_t *serialized_data = msg.SerializeData(serialized_size);
		int32_t sent;
		if (c->GetClient()->Send(serialized_data, serialized_size, sent))
		{
			//couldnt send
		}

		m_queue->EnqueueConnection(msg);
	}

	uint32_t TcpConnectionHandler::GetAvailableID()
	{
		for (int i = 1; i <= m_maxConnections; i++)
		{
			bool flag = true;
			m_listMutex.lock();
			for (int k = 0; k < m_list.size(); k++)
			{
				if (m_list.at(k)->GetID() == i)
					flag = false;
			}
			m_listMutex.unlock();

			if (flag)
				return i;
		}

		//throw OutOfRangeException("Out of IDs to allocate - clients = max connections", "NewConnectionEventPool");
		return -1;
	}

	void TcpConnectionHandler::HandleReceive()
	{
		for (int i = 0; i < m_list.size(); i++)
		{
			std::shared_ptr<TcpConnection> c = m_list.at(i);
			std::unique_ptr<uint8_t> header(new uint8_t[sizeof(NetworkHeader*)]());

			int32_t read;
			if (!c->GetClient()->Recv(header.get(), sizeof(NetworkHeader*), read))
				return;

			if (read == sizeof(NetworkHeader*))
			{
				std::unique_ptr<NetworkHeader> net_header((NetworkHeader*)header.get());

				std::unique_ptr<uint8_t> buffer(new uint8_t[net_header->Size]());
				int32_t read;
				if (!c->GetClient()->Recv(buffer.get(), net_header->Size, read))
				{
					if (read != net_header->Size)
						return; // wrong message?

					NetworkMessage msg;
					msg.Deserialize(buffer.get(), net_header->Size);

					if (msg.m_tag == (uint32_t)InternalTags::Disconnect)
						m_queue->EnqueueDisconnection(msg);
					else if (msg.m_tag == (uint32_t)InternalTags::Connect)
						m_queue->EnqueueConnection(msg);
					else
						m_queue->EnqueueMessageReceived(msg);
				}
			}
			else // wrong message
			{
				return;
			}
		}
	}

	void TcpConnectionHandler::HandleSend()
	{
		if (m_queue->SendSize() > 0)
		{
			NetworkMessage msg = m_queue->DequeueMessageToSend();

			uint32_t size;
			std::unique_ptr<uint8_t> data(msg.SerializeData(size));

			if (msg.m_distributionMode == DistributionMode::Others)
			{
				m_listMutex.lock();
				for (int i = 0; i < m_list.size(); i++)
				{
					std::shared_ptr<TcpConnection> c = m_list.at(i);
					if (c->GetID() != msg.m_senderID)
					{
						int32_t sent;
						if (!c->GetClient()->Send(data.get(), size, sent))
						{
							// it failed - retry? or just disconnect right in the first try
						}
					}
				}
				m_listMutex.unlock();
			}
			else if (msg.m_distributionMode == DistributionMode::OthersAndServer)
			{
				m_listMutex.lock();
				for (int i = 0; i < m_list.size(); i++)
				{
					std::shared_ptr<TcpConnection> c = m_list.at(i);
					if (c->GetID() != msg.m_senderID)
					{
						int32_t sent;
						if (!c->GetClient()->Send(data.get(), size, sent))
						{
							// it failed - retry? or just disconnect right in the first try
						}
					}
				}
				m_listMutex.unlock();

				//handle to plugins too
			}
			else if (msg.m_distributionMode == DistributionMode::ID)
			{
				m_listMutex.lock();
				for (int i = 0; i < m_list.size(); i++)
				{
					std::shared_ptr<TcpConnection> c = m_list.at(i);
					if (c->GetID() == msg.m_senderID)
					{
						int32_t sent;
						if (!c->GetClient()->Send(data.get(), size, sent))
						{
							// it failed - retry? or just disconnect right in the first try
						}
					}
				}
				m_listMutex.unlock();
			}
			else if (msg.m_distributionMode == DistributionMode::All)
			{
				m_listMutex.lock();
				for (int i = 0; i < m_list.size(); i++)
				{
					std::shared_ptr<TcpConnection> c = m_list.at(i);

					int32_t sent;
					if (!c->GetClient()->Send(data.get(), size, sent))
					{
						// it failed - retry? or just disconnect right in the first try
					}
				}
				m_listMutex.unlock();
			}
			else if (msg.m_distributionMode == DistributionMode::AllAndMe)
			{
				m_listMutex.lock();
				for (int i = 0; i < m_list.size(); i++)
				{
					std::shared_ptr<TcpConnection> c = m_list.at(i);
						
					int32_t sent;
					if (!c->GetClient()->Send(data.get(), size, sent))
					{
						// it failed - retry? or just disconnect right in the first try
					}
				}
				m_listMutex.unlock();

				//handle to plugins too
			}
			else if (msg.m_distributionMode == DistributionMode::Server)
			{
				//handle just in plugins
			}
		}
	}

	void TcpConnectionHandler::HandleReceiveThreaded()
	{
		while (m_run.load())
		{
			HandleReceive();
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}

	void TcpConnectionHandler::HandleSendThreaded()
	{
		while (m_run.load())
		{
			HandleSend();
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
}