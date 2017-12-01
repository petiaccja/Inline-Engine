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
	}

	TcpListener(Socket *InSocket, std::chrono::milliseconds inSleepTime = std::chrono::milliseconds(1))
		: m_deleteSocket(false)
		, m_sleepTime(inSleepTime)
		, m_socket(InSocket)
		, m_stopping(false)
	{
	}

	~TcpListener()
	{
		Stop();
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
		std::thread acceptor_thread(&TcpListener::AcceptClients, this);
		m_acceptor_thread.swap(acceptor_thread);

		std::thread receiver_thread(&TcpListener::HandleClients, this);
		m_receiver_thread.swap(receiver_thread);
	}

	void SetDispatcher(NetworkDispatcher *dispatcher)
	{
		m_dispatcher = dispatcher;
	}

private:
	void AcceptClients()
	{
		if (m_socket == nullptr)
		{
			m_socket = TcpSocketBuilder().AsReusable().BoundToPort(m_port).Listening().WithSendBufferSize(64);
		}

		if (m_socket != nullptr)
			return;

		std::string remoteAddress;

		const bool hasZeroSleepTime = (m_sleepTime == std::chrono::milliseconds(0));

		while (!m_stopping)
		{
			bool pending = false;

			if (m_socket->WaitForPendingConnection(pending, m_sleepTime))
			{
				if (pending)
				{
					Socket* connectionSocket = (Socket*)m_socket->Accept();

					if (connectionSocket != nullptr)
					{
						m_connections.emplace_back(connectionSocket);
					}
				}
				else if (hasZeroSleepTime)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(0));
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(m_sleepTime));
			}
		}
	}

	void HandleClients()
	{
		if (!m_dispatcher)
		{
			exit(0);
		}

		while (!m_stopping)
		{
			if (!(m_connections.size() > 0))
				continue;

			for (int i = 0; i < m_connections.size(); i++)
			{
				Socket *currentConnection = m_connections.at(i);

				uint32_t dataSize;
				if (currentConnection->HasPendingData(dataSize))
				{
					uint8_t *data;
					int32_t read;
					if (currentConnection->Recv(data, dataSize, read))
					{
						m_dispatcher->Enqueue(std::string((char*)data, read));
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(m_sleepTime));
			}
		}
	}

public:
	virtual void Stop()
	{
		m_stopping = true;
	}

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