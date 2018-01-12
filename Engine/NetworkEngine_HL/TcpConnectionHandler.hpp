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
		~TcpConnectionHandler();

		void Start();
		void Stop();
		void AddClient(std::shared_ptr<TcpConnection> &c);
		void SetMaxConnections(uint32_t max_connections);

		uint32_t GetAvailableID();

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