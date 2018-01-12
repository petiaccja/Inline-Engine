#pragma once

#include <mutex>
#include <atomic>
#include <queue>

#include <BaseLibrary/SpinMutex.hpp>

namespace inl::net
{
	class MessageQueue;
}

namespace inl::net::servers
{
	class TcpConnection;

	class TcpConnectionHandler
	{
	public:
		TcpConnectionHandler();

		inline ~TcpConnectionHandler()
		{
			m_run.exchange(false);
		}

		void Start();

		inline void Stop()
		{
			m_run.exchange(false);
		}

		void Add(std::shared_ptr<TcpConnection> &c);

		uint32_t GetAvailableID();

		inline void SetMaxConnections(uint32_t max_connections)
		{
			m_maxConnections = max_connections;
		}

		std::shared_ptr<MessageQueue> m_queue; // quick hack

	private:
		void HandleReceive();
		void HandleSend();

		void HandleReceiveThreaded();
		void HandleSendThreaded();

	private:
		std::vector<std::shared_ptr<TcpConnection>> m_list;
		uint32_t m_maxConnections;

		std::thread m_receiveThread;
		std::thread m_sendThread;

		std::atomic_bool m_run;

		inl::spin_mutex m_listMutex;
	};
}