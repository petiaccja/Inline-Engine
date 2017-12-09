#include "TcpSocketBuilder.hpp"

Socket * TcpSocketBuilder::Build() const
{
	Socket* socket = new Socket(SocketType::Streaming, m_socketProtocol);

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
		{
			delete socket;
			socket = nullptr;
			throw std::exception("Couldnt create socket");
		}
	}

	return socket;
}
