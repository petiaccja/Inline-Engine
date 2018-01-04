#include "TcpSocketBuilder.hpp"

#include "TcpClient.hpp"
#include "TcpListener.hpp"

namespace inl::net::sockets
{
	std::shared_ptr<Socket> TcpSocketBuilder::Build() const
	{
		std::shared_ptr<Socket> socket = std::shared_ptr<Socket>(new Socket(SocketType::Streaming, m_socketProtocol));

		if (socket != nullptr)
		{
			bool Error = !socket->SetReuseAddr(m_reusable) ||
				!socket->SetLinger(m_linger, m_lingerTimeout);

			if (!Error)
			{
				Error = m_bound && !socket->Bind(m_boundAddr);
			}

			if (!Error)
			{
				Error = m_listen && !socket->Listen();
			}

			if (!Error)
			{
				Error = !socket->SetNonBlocking(!m_blocking);
			}

			if (!Error)
			{
				int32_t OutNewSize;

				if (m_receiveBufferSize > 0)
				{
					socket->SetReceiveBufferSize(m_receiveBufferSize, OutNewSize);
				}

				if (m_sendBufferSize > 0)
				{
					socket->SetSendBufferSize(m_sendBufferSize, OutNewSize);
				}
			}

			if (Error)
				throw inl::RuntimeException("Couldnt create socket"); // make parameter a string depending on the error
		}

		return socket;
	}

	std::shared_ptr<TcpClient> TcpSocketBuilder::BuildClient() const
	{
		std::shared_ptr<Socket> socket = Build();
		return std::shared_ptr<TcpClient>(new TcpClient(socket));
	}

	std::shared_ptr<TcpListener> TcpSocketBuilder::BuildListener() const
	{
		std::shared_ptr<Socket> socket = Build();
		return std::shared_ptr<TcpListener>(new TcpListener(socket));
	}
}