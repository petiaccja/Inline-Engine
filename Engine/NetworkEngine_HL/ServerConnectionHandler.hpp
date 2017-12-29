#pragma once

#include <queue>
#include <map>

#include <Exception/Exception.hpp>
#include <Serialization/BinarySerializer.hpp>

#include <TcpClient.hpp>

#include "NewConnectionEvent.hpp"
#include "DisconnectedEvent.hpp"
#include "ServerConnection.hpp"
#include "NetworkMessage.hpp"

namespace inl::net::servers
{
	using namespace sockets;
	using namespace events;

	class ServerConnectionHandler
	{
	public:
		ServerConnectionHandler(bool multithreaded = true)
		{
			m_run = true;

			if (multithreaded)
			{
				std::thread receive_thread(&ServerConnectionHandler::HandleReceive, this);
				m_receiveThread.swap(receive_thread);

				std::thread send_thread(&ServerConnectionHandler::HandleSend, this);
				m_sendThread.swap(send_thread);
			}
		}

		~ServerConnectionHandler()
		{
			m_run.exchange(false);
		}

		void Stop()
		{
			m_run.exchange(false);
		}

		void Add(ServerConnection *c)
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

		uint32_t GetAvailableID()
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

		void SetMaxConnections(uint32_t max_connections)
		{
			m_maxConnections = max_connections;
		}

	private:
		void HandleReceive()
		{
			while (m_run.load())
			{

			}
		}

		void HandleSend()
		{
			while (m_run.load())
			{
				if (m_messagesToSend.size() > 0)
				{
					m_sendMutex.lock();
					NetworkMessage msg = m_messagesToSend.front();
					m_messagesToSend.pop();
					m_sendMutex.unlock();

					uint32_t count;
					uint8_t* data = msg.SerializeData(count);

					if (msg.DistributionMode == DistributionMode::Others)
					{
						for (int i = 0; i < m_list.size(); i++) // should i lock here?
						{
							ServerConnection *c = m_list.at(i);
							if (c->GetID() != msg.SenderID)
							{
								int32_t sent;
								if (!c->GetClient()->Send(data, count, sent))
								{
									// it failed - retry? or just disconnect right in the first try
								}
							}
						}
					}
				}
			}
		}

		void HandleSendReceive() // no loop
		{
			if (m_messagesToSend.size() > 0)
			{
				m_sendMutex.lock();
				NetworkMessage msg = m_messagesToSend.front();
				m_messagesToSend.pop();
				m_sendMutex.unlock();

				uint32_t count;
				uint8_t* data = msg.SerializeData(count);

				if (msg.DistributionMode == DistributionMode::Others)
				{
					for (int i = 0; i < m_list.size(); i++) // should i lock here?
					{
						ServerConnection *c = m_list.at(i);
						if (c->GetID() != msg.SenderID)
						{
							int32_t sent;
							if (!c->GetClient()->Send(data, count, sent))
							{
								// it failed - retry? or just disconnect right in the first try
							}
						}
					}
				}
			}

			//receive
		}

	private:
		std::vector<ServerConnection*> m_list;
		uint32_t m_maxConnections;

		std::thread m_receiveThread;
		std::thread m_sendThread;

		std::atomic_bool m_run;

		std::queue<NetworkMessage> m_messagesToSend;
		std::queue<NetworkMessage> m_receivedMessages;

		std::mutex m_sendMutex;
		std::mutex m_receiveMutex;
	};
}