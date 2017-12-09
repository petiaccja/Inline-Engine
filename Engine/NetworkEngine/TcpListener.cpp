#include "TcpListener.hpp"

void TcpListener::Start()
{
	std::thread thread(&TcpListener::AcceptClients, this);
	m_thread.swap(thread);
}

void TcpListener::AcceptClients()
{
	if (m_socket == nullptr)
	{
		m_socket = TcpSocketBuilder().AsReusable().Bind(IPAddress(0, 0, 0, 0, m_port)).Listening().WithSendBufferSize(64);
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
					//m_connections.emplace_back(connectionSocket);
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
