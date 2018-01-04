#pragma once

#include <mutex>
#include <atomic>
#include <queue>

#include "NetworkMessage.hpp"
#include "DataReceivedEvent.hpp"

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

	private:
		std::vector<std::shared_ptr<ServerConnection>> m_list;
		uint32_t m_maxConnections;

		std::thread m_receiveThread;
		std::thread m_sendThread;

		std::atomic_bool m_run;

		std::queue<NetworkMessage> m_messagesToSend;
		std::queue<events::DataReceivedEvent> m_receivedMessages;

		std::mutex m_sendMutex;
		std::mutex m_receiveMutex;
		std::mutex m_listMutex;
	};
}