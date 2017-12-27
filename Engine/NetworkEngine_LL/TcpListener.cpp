#include "TcpListener.hpp"
#include "TcpClient.hpp"

namespace inl::net::sockets
{
	TcpClient *TcpListener::AcceptClient()
	{
		if (m_socket == nullptr)
		{
			m_socket = TcpSocketBuilder().AsReusable().Bind(IPAddress(0, 0, 0, 0, m_port)).Listening().Build();
		}

		if (m_socket == nullptr)
			return nullptr;

		std::string remoteAddress;

		const bool hasZeroSleepTime = (m_sleepTime == std::chrono::milliseconds(0));

		bool pending = false;

		if (m_socket->WaitForPendingConnection(pending, m_sleepTime))
		{
			if (pending)
			{
				Socket* connectionSocket = (Socket*)m_socket->Accept();

				if (connectionSocket != nullptr)
				{
					return new TcpClient(connectionSocket);
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

		return nullptr;
	}
}