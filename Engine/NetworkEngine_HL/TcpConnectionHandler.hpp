#pragma once

#include <mutex>
#include <atomic>
#include <queue>

#include <BaseLibrary/SpinMutex.hpp>

namespace inl::net
{
	class MessageQueue;
	class TcpConnection;
	class Server;
}

namespace inl::net::servers
{
	class TcpConnectionHandler
	{
		friend class inl::net::Server;

	public:
		TcpConnectionHandler();
		~TcpConnectionHandler();

		void Start();
		void Stop();
		void AddClient(std::shared_ptr<TcpConnection> &c);
		void SetMaxConnections(uint32_t max_connections);

		uint32_t GetAvailableID();

	private:
		void HandleReceive();
		void HandleSend();

		void HandleReceiveThreaded();
		void HandleSendThreaded();

	private:
		std::vector<std::shared_ptr<TcpConnection>> m_list;
		inl::spin_mutex m_listMutex;

		uint32_t m_maxConnections;

		std::thread m_receiveThread;
		std::thread m_sendThread;

		std::atomic_bool m_run;

		std::shared_ptr<MessageQueue> m_queue;
	};
}