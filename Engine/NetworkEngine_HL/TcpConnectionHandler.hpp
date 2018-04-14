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

	namespace sockets
	{
		class TcpListener;
	}
}

namespace inl::net::servers
{
	class TcpConnectionHandler
	{
		friend class inl::net::Server;

	public:
		TcpConnectionHandler(std::shared_ptr<inl::net::sockets::TcpListener> listener_ptr);
		~TcpConnectionHandler();

		void Start();
		void Stop();
		void AddClient(std::shared_ptr<TcpConnection> &c);
		void SetMaxConnections(uint32_t max_connections);

		uint32_t GetAvailableID();

	private:
		void HandleReceiveMsgAndConns();
		void HandleSend();

		void HandleReceiveMsgAndConnsThreaded();
		void HandleSendThreaded();

	private:
		std::vector<std::shared_ptr<TcpConnection>> m_list;
		inl::SpinMutex m_listMutex;

		uint32_t m_maxConnections;

		std::thread m_receiveThread;
		std::thread m_sendThread;

		std::atomic_bool m_run;

		std::shared_ptr<MessageQueue> m_queue;

		std::shared_ptr<inl::net::sockets::TcpListener> m_listenerPtr;
	};
}