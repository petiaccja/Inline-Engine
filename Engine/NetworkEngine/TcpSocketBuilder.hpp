#pragma once

#include "Net.hpp"
#include "Socket.hpp"

class TcpSocketBuilder
{
public:
	TcpSocketBuilder() :
		m_blocking(false)
		, m_bound(false)
		, m_boundPort(0)
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

	TcpSocketBuilder BoundToPort(int32_t Port)
	{
		m_boundPort = Port;
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

	Socket* Build() const;

private:
	bool m_blocking;
	bool m_bound;
	int32_t m_boundPort;
	bool m_linger;
	int32_t m_lingerTimeout;
	bool m_listen;
	int32_t m_receiveBufferSize;
	bool m_reusable;
	int32_t m_sendBufferSize;

	SocketProtocol m_socketProtocol;
};
