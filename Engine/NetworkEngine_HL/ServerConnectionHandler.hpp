#pragma once

#include <mutex>
#include <atomic>
#include <queue>

#include "NetworkMessage.hpp"

namespace inl::net::servers
{
	class ServerConnection;

	class ServerConnectionHandler
	{
	public:
		ServerConnectionHandler(bool multithreaded = true);

		inline ~ServerConnectionHandler()
		{
			m_run.exchange(false);
		}

		inline void Stop()
		{
			m_run.exchange(false);
		}

		void Add(std::shared_ptr<ServerConnection> c);

		uint32_t GetAvailableID();

		inline void SetMaxConnections(uint32_t max_connections)
		{
			m_maxConnections = max_connections;
		}

	private:
		void HandleReceive();
		void HandleSend();

		void HandleReceiveThreaded();
		void HandleSendThreaded();

		//void HandleSendReceive() // no loop
		//{
		//	if (m_messagesToSend.size() > 0)
		//	{
		//		m_sendMutex.lock();
		//		NetworkMessage msg = m_messagesToSend.front();
		//		m_messagesToSend.pop();
		//		m_sendMutex.unlock();

		//		uint32_t count;
		//		uint8_t* data = msg.SerializeData(count);

		//		if (msg.m_distributionMode == DistributionMode::Others)
		//		{
		//			for (int i = 0; i < m_list.size(); i++) // should i lock here?
		//			{
		//				ServerConnection *c = m_list.at(i);
		//				if (c->GetID() != msg.m_senderID)
		//				{
		//					int32_t sent;
		//					if (!c->GetClient()->Send(data, count, sent))
		//					{
		//						// it failed - retry? or just disconnect right in the first try
		//					}
		//				}
		//			}
		//		}
		//	}

		//	//receive
		//}

	private:
		std::vector<std::shared_ptr<ServerConnection>> m_list;
		uint32_t m_maxConnections;

		std::thread m_receiveThread;
		std::thread m_sendThread;

		std::atomic_bool m_run;

		std::queue<NetworkMessage> m_messagesToSend;
		std::queue<NetworkMessage> m_receivedMessages;

		std::mutex m_sendMutex;
		std::mutex m_receiveMutex;
		std::mutex m_listMutex;
	};
}