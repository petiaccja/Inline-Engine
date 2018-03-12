#include "TcpConnectionHandler.hpp"

#include "DisconnectedEvent.hpp"
#include "NewConnectionEvent.hpp"
#include "InternalTags.hpp"

#include "NetworkMessage.hpp"
#include "MessageQueue.hpp"
#include "TcpConnection.hpp"
#include "NetworkEngine_LL/TcpListener.hpp"

#include <chrono>

namespace inl::net::servers
{
	using namespace events;

	TcpConnectionHandler::TcpConnectionHandler(std::shared_ptr<TcpListener> listener_ptr)
		: m_run(false)
		, m_listenerPtr(listener_ptr)
	{
	}

	TcpConnectionHandler::~TcpConnectionHandler()
	{
		m_run.exchange(false);
	}

	void TcpConnectionHandler::Start()
	{
		m_run.exchange(true);
		std::thread receive_thread(&TcpConnectionHandler::HandleReceiveMsgAndConnsThreaded, this);
		m_receiveThread.swap(receive_thread);

		//std::thread send_thread(&TcpConnectionHandler::HandleSendThreaded, this);
		//m_sendThread.swap(send_thread);
	}

	void TcpConnectionHandler::Stop()
	{
		m_run.exchange(false);
	}

	void TcpConnectionHandler::AddClient(std::shared_ptr<TcpConnection> &c)
	{
		uint32_t id = GetAvailableID();
		if (id == -1)
		{
			// this can be handled just by the server
			// what if the server owner wants to know if a user wanted to join but couldnt
			DisconnectedEvent disconnected_event(id, "Server Full", -1);
			std::shared_ptr<TcpClient> client = c->GetClient();
			/*int32_t size = 0;
			uint8_t *buffer = disconnected_event.Serialize(size);
			int32_t sent = 0;
			client->Send(buffer, size, sent);*/
			client->Close();
		}

		c->SetID(id);

		NetworkMessage msg(id, DistributionMode::ID, id, (uint32_t)InternalTags::AssignID, &id, sizeof(uint32_t));

		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		uint32_t serialized_size;
		uint8_t *serialized_data = msg.SerializeData<uint32_t>(serialized_size);
		int32_t sent;
		if (!c->GetClient()->Send(serialized_data, serialized_size, sent))
		{
			//couldnt send
			return;
		}

		m_listMutex.lock();
		m_list.push_back(c);
		m_listMutex.unlock();

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

	void TcpConnectionHandler::SetMaxConnections(uint32_t max_connections)
	{
		m_maxConnections = max_connections;
	}

	void TcpConnectionHandler::HandleReceiveMsgAndConns()
	{
		// https://www.ibm.com/support/knowledgecenter/en/ssw_i5_54/rzab6/poll.htm
		std::vector<pollfd> poll_fds;
		pollfd master_fd;
		master_fd.fd = m_listenerPtr->m_socket->GetNativeSocket();
		master_fd.events = POLLRDNORM;
		poll_fds.emplace_back(master_fd);

		int res = poll(poll_fds.data(), poll_fds.size(), -1);

		if (res < 0)
		{
			//poll error
		}

		//should never timeout because its infinite (negative)
		//if (res == 0)
		//{
			//timeout
		//}

		for (int i = 0; i < poll_fds.size(); i++)
		{
			if (poll_fds.at(i).revents == 0)
				continue;

			if (poll_fds[i].revents != POLLRDNORM)
			{
				printf("  Error! revents = %d\n", poll_fds.at(i).revents);
				//end_server = TRUE;
				break;

			}
			if (poll_fds.at(i).fd == m_listenerPtr->m_socket->GetNativeSocket())
			{
				TcpClient *c = m_listenerPtr->AcceptClient();
				if (c)
				{
					std::shared_ptr<TcpConnection> connection = std::make_shared<TcpConnection>(c);

					pollfd new_client;
					new_client.fd = connection->GetClient()->m_socket->GetNativeSocket();
					new_client.events = POLLRDNORM;
					poll_fds.emplace_back(new_client);

					AddClient(connection);
				}
			}
			else // not the listening socket
			{
				SOCKET c = poll_fds.at(i).fd;
				std::unique_ptr<uint8_t> header(new uint8_t[sizeof(NetworkHeader*)]());

				int32_t read;
				if ((read = recv(c, (char*)header.get(), sizeof(NetworkHeader*), 0)) != sizeof(NetworkHeader*))
					continue;

				std::unique_ptr<NetworkHeader> net_header((NetworkHeader*)header.get());
				std::unique_ptr<uint8_t> buffer(new uint8_t[net_header->Size]());

				if ((read = recv(c, (char*)buffer.get(), net_header->Size, 0)) == net_header->Size)
				{
					NetworkMessage msg;
					msg.Deserialize(buffer.get(), net_header->Size);

					if (msg.GetTag() == (uint32_t)InternalTags::Disconnect)
						m_queue->EnqueueDisconnection(msg);
					else if (msg.GetTag() == (uint32_t)InternalTags::Connect)
						m_queue->EnqueueConnection(msg);
					else
						m_queue->EnqueueMessageReceived(msg);
				}
				else
					continue;
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

			if (msg.GetDistributionMode() == DistributionMode::Others)
			{
				m_listMutex.lock();
				for (int i = 0; i < m_list.size(); i++)
				{
					std::shared_ptr<TcpConnection> c = m_list.at(i);
					if (c->GetID() != msg.GetSenderID())
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
			else if (msg.GetDistributionMode() == DistributionMode::OthersAndServer)
			{
				m_listMutex.lock();
				for (int i = 0; i < m_list.size(); i++)
				{
					std::shared_ptr<TcpConnection> c = m_list.at(i);
					if (c->GetID() != msg.GetSenderID())
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
			else if (msg.GetDistributionMode() == DistributionMode::ID)
			{
				m_listMutex.lock();
				for (int i = 0; i < m_list.size(); i++)
				{
					std::shared_ptr<TcpConnection> c = m_list.at(i);
					if (c->GetID() == msg.GetSenderID())
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
			else if (msg.GetDistributionMode() == DistributionMode::All)
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
			else if (msg.GetDistributionMode() == DistributionMode::AllAndMe)
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
			else if (msg.GetDistributionMode() == DistributionMode::Server)
			{
				//handle just in plugins
			}
		}
	}

	void TcpConnectionHandler::HandleReceiveMsgAndConnsThreaded()
	{
		while (m_run.load())
		{
			HandleReceiveMsgAndConns();
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