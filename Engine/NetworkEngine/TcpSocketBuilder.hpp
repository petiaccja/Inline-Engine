#pragma once

#include "Net.hpp"
#include "Socket.hpp"

class TcpSocketBuilder
{
public:
	TcpSocketBuilder() :
		m_blocking(false)
		, m_bound(false)
		, m_boundAddr(IPAddress::Any)
		, m_linger(false)
		, m_lingerTimeout(0)
		, m_listen(false)
		, m_receiveBufferSize(0)
		, m_reusable(false)
		, m_sendBufferSize(0)
		, m_socketProtocol(SocketProtocol::IPv4)
	{ 
	}

	TcpSocketBuilder AsBlocking()
	{
		m_blocking = true;

		return *this;
	}

	TcpSocketBuilder AsNonBlocking()
	{
		m_blocking = false;

		return *this;
	}

	TcpSocketBuilder AsReusable()
	{
		m_reusable = true;

		return *this;
	}

	TcpSocketBuilder Bind(const IPAddress &addr)
	{
		m_boundAddr = addr;
		m_bound = true;

		return *this;
	}

	TcpSocketBuilder Lingering(int32_t Timeout)
	{
		m_linger = true;
		m_lingerTimeout = Timeout;

		return *this;
	}

	TcpSocketBuilder Listening()
	{
		m_listen = true;

		return *this;
	}

	TcpSocketBuilder WithReceiveBufferSize(int32_t SizeInBytes)
	{
		m_receiveBufferSize = SizeInBytes;

		return *this;
	}

	TcpSocketBuilder WithSendBufferSize(int32_t SizeInBytes)
	{
		m_sendBufferSize = SizeInBytes;

		return *this;
	}

	TcpSocketBuilder Protocol(SocketProtocol prot)
	{
		m_socketProtocol = prot;

		return *this;
	}

public:
	operator Socket*() const
	{
		return Build();
	}

	Socket* Build() const
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

private:
	bool m_blocking;
	bool m_bound;
	IPAddress m_boundAddr;
	bool m_linger;
	int32_t m_lingerTimeout;
	bool m_listen;
	int32_t m_receiveBufferSize;
	bool m_reusable;
	int32_t m_sendBufferSize;

	SocketProtocol m_socketProtocol;
};
