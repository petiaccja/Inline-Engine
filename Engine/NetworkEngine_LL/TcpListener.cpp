#include "TcpListener.hpp"
#include "TcpSocketBuilder.hpp"
#include "Socket.hpp"
#include "TcpClient.hpp"

namespace inl::net::sockets
{
	TcpListener::TcpListener(uint16_t port, std::chrono::milliseconds inSleepTime)
		: m_port(port)
		, m_sleepTime(inSleepTime)
	{
		m_socket = TcpSocketBuilder().AsNonBlocking().AsReusable().Bind(IPAddress(0, 0, 0, 0, port)).Listening().Build();
	}

	TcpListener::TcpListener(Socket *InSocket, std::chrono::milliseconds inSleepTime)
		: m_sleepTime(inSleepTime)
	{
		m_socket = std::unique_ptr<Socket>(InSocket);
	}

	TcpClient *TcpListener::AcceptClient()
	{
		if (m_socket == nullptr)
			m_socket = TcpSocketBuilder().AsReusable().Bind(IPAddress(0, 0, 0, 0, m_port)).Listening().Build();

		if (m_socket == nullptr)
			return nullptr;

		std::string remoteAddress;

		const bool hasZeroSleepTime = (m_sleepTime == std::chrono::milliseconds(0));

		bool pending = false;

		if (m_socket->WaitForPendingConnection(pending, m_sleepTime))
		{
			if (pending)
			{
				std::unique_ptr<Socket> connectionSocket = m_socket->Accept();

				if (connectionSocket != nullptr)
				{
					return new TcpClient(connectionSocket.release());
				}
			}
			else if (hasZeroSleepTime)
				std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(m_sleepTime));

		return nullptr;
	}
}