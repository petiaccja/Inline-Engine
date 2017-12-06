#include "ISocket.hpp"

bool ISocket::SendTo(const uint8_t* data, int32_t count, int32_t& sent, const IPAddress& addrDest)
{
	throw std::exception("Not implemented");
}

bool ISocket::Send(const uint8_t* data, int32_t count, int32_t& sent)
{
	throw std::exception("Not implemented");
}

bool ISocket::RecvFrom(uint8_t* data, int32_t size, int32_t& read, IPAddress& srcAddr, SocketReceiveFlags flags)
{
	throw std::exception("Not implemented");
}

bool ISocket::Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags)
{
	throw std::exception("Not implemented");
}