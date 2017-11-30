#include "ISocket.hpp"

bool ISocket::SendTo(const uint8_t* data, int32_t count, int32_t& sent, const std::string& addrDest)
{
	return true;
}

bool ISocket::Send(const uint8_t* data, int32_t count, int32_t& sent)
{
	return true;
}

bool ISocket::RecvFrom(uint8_t* data, int32_t size, int32_t& read, std::string& srcAddr, SocketReceiveFlags flags)
{
	return true;
}

bool ISocket::Recv(uint8_t* data, int32_t size, int32_t& read, SocketReceiveFlags flags)
{
	return true;
}