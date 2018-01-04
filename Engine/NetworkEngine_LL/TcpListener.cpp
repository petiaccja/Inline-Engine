#include "TcpListener.hpp"
#include "TcpClient.hpp"

namespace inl::net::sockets
{
	std::shared_ptr<TcpClient> TcpListener::AcceptClient()
	{
		if (m_socket == nullptr)
		{
			m_socket = std::unique_ptr<Socket>(&*(TcpSocketBuilder().AsReusable().Bind(IPAddress(0, 0, 0, 0, m_port)).Listening().Build()));
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
				std::shared_ptr<Socket> connectionSocket = m_socket->Accept();

				if (connectionSocket != nullptr)
				{
					return std::shared_ptr<TcpClient>(new TcpClient(connectionSocket));
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