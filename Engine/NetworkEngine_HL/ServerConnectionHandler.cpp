#include "ServerConnectionHandler.hpp"

#include "ServerConnection.hpp"
#include "DisconnectedEvent.hpp"
#include "NewConnectionEvent.hpp"

namespace inl::net::servers
{
	using namespace events;

	ServerConnectionHandler::ServerConnectionHandler(bool multithreaded)
		: m_run(true)
	{
		if (multithreaded)
		{
			std::thread receive_thread(&ServerConnectionHandler::HandleReceive, this);
			m_receiveThread.swap(receive_thread);

			std::thread send_thread(&ServerConnectionHandler::HandleSend, this);
			m_sendThread.swap(send_thread);
		}
	}

	void ServerConnectionHandler::Add(ServerConnection * c)
	{
		uint32_t id = GetAvailableID();
		if (id == -1)
		{
			// this can be handled just by the server
			// what if the server owner wants to know if a user wanted to join but couldnt
			DisconnectedEvent disconnected_event(id, "Server Full", -1);
			TcpClient *client = c->GetClient();
			int32_t size = 0;
			uint8_t *buffer = disconnected_event.Serialize(size);
			int32_t sent = 0;
			client->Send(buffer, size, sent);
			client->Close();
		}

		c->SetID(id);

		m_list.push_back(c);
		NewConnectionEvent *new_conn_event = new NewConnectionEvent(c->GetClient(), id);
		// send new connection event to main thread - but how
	}

	uint32_t ServerConnectionHandler::GetAvailableID()
	{
		for (int i = 1; i <= m_maxConnections; i++)
		{
			bool flag = true;
			for (int k = 0; k < m_list.size(); k++)
			{
				if (m_list.at(k)->GetID() == i)
					flag = false;
			}

			if (flag)
				return i;
		}

		//throw OutOfRangeException("Out of IDs to allocate - clients = max connections", "NewConnectionEventPool");
		return -1;
	}

	void ServerConnectionHandler::HandleReceive()
	{
		while (m_run.load())
		{

		}
	}

	void ServerConnectionHandler::HandleSend()
	{
		while (m_run.load())
		{
			if (m_messagesToSend.size() > 0)
			{
				m_sendMutex.lock();
				NetworkMessage msg = m_messagesToSend.front();
				m_messagesToSend.pop();
				m_sendMutex.unlock();

				uint32_t size;
				uint8_t* data = msg.SerializeData(size);

				if (msg.m_distributionMode == DistributionMode::Others)
				{
					for (int i = 0; i < m_list.size(); i++) // should i lock here?
					{
						ServerConnection *c = m_list.at(i);
						if (c->GetID() != msg.m_senderID)
						{
							int32_t sent;
							if (!c->GetClient()->Send(data, size, sent))
							{
								// it failed - retry? or just disconnect right in the first try
							}
						}
					}
				}
			}
		}
	}
}