#pragma once

#include "Socket.hpp"
#include "TcpSocketBuilder.hpp"
#include "NetworkDispatcher.hpp"

#include <thread>

class TcpListener
{
public:
	TcpListener(int16_t port, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1))
		: m_port(port)
		, m_deleteSocket(true)
		, m_sleepTime(inSleepTime)
		, m_socket(nullptr)
		, m_stopping(false)
	{
		//Thread = FRunnableThread::Create(this, TEXT("FTcpListener"), 8 * 1024, TPri_Normal);
	}

	TcpListener(Socket *InSocket, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1))
		: m_deleteSocket(false)
		, m_sleepTime(inSleepTime)
		, m_socket(InSocket)
		, m_stopping(false)
	{
		//Thread = FRunnableThread::Create(this, TEXT("FTcpListener"), 8 * 1024, TPri_Normal);
	}

	~TcpListener()
	{
		if (Thread != nullptr)
		{
			Thread->Kill(true);
			delete Thread;
		}

		if (m_deleteSocket && (m_socket != nullptr))
		{
			delete m_socket;
			m_socket = nullptr;
		}
	}

public:
	Socket* GetSocket() const
	{
		return m_socket;
	}

	bool IsActive() const
	{
		return ((m_socket != nullptr) && !m_stopping);
	}

	void Start()
	{
		std::thread acceptor_thread(TcpListener::AcceptClients, this);
		m_acceptor_thread.swap(acceptor_thread);

		std::thread receiver_thread(TcpListener::HandleClients, this);
		m_receiver_thread.swap(receiver_thread);
	}

	void SetDispatcher(NetworkDispatcher *dispatcher)
	{
		m_dispatcher = dispatcher;
	}

public:
	static void AcceptClients(TcpListener *listener)
	{
		if (listener->m_socket == nullptr)
		{
			listener->m_socket = TcpSocketBuilder().AsReusable().BoundToPort(listener->m_port).Listening().WithSendBufferSize(64);
		}

		if (listener->m_socket != nullptr)
			return;

		std::string remoteAddress;

		const bool hasZeroSleepTime = (listener->m_sleepTime == std::chrono::milliseconds(0));

		while (!listener->m_stopping)
		{
			bool pending = false;

			if (listener->m_socket->WaitForPendingConnection(pending, listener->m_sleepTime))
			{
				if (pending)
				{
					Socket* connectionSocket = (Socket*)listener->m_socket->Accept();

					if (connectionSocket != nullptr)
					{
						listener->m_connections.emplace_back(connectionSocket);
					}
				}
				else if (hasZeroSleepTime)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(0));
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(listener->m_sleepTime));
			}
		}
	}

	static void HandleClients(TcpListener *listener)
	{
		if (!listener->m_dispatcher)
		{
			exit(0);
		}

		while (!listener->m_stopping)
		{
			if (!(listener->m_connections.size() > 0))
				continue;

			for (int i = 0; i < listener->m_connections.size(); i++)
			{
				Socket *currentConnection = listener->m_connections.at(i);

				uint32_t dataSize;
				if (currentConnection->HasPendingData(dataSize))
				{
					uint8_t *data;
					int32_t read;
					if (currentConnection->Recv(data, dataSize, read))
					{
						listener->m_dispatcher->Enqueue(std::string((char*)data, read));
					}
				}
			}
		}
	}

	virtual void Stop()
	{
		m_stopping = true;
	}

	virtual void Exit() { }

private:

	std::vector<Socket*> m_connections;

	bool m_deleteSocket;
	std::chrono::milliseconds m_sleepTime;
	Socket* m_socket;
	bool m_stopping;
	int16_t m_port;
	
	NetworkDispatcher *m_dispatcher;

	std::thread m_acceptor_thread;
	std::thread m_receiver_thread;
};