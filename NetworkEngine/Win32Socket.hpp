#pragma once

#include "SocketImpl.hpp"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

namespace inl::net
{
	class Win32Socket : public SocketImpl
	{
	public:
		inline Win32Socket() : SocketImpl() { }
		Win32Socket(enums::Type type);

		void Close();
		bool Bind(uint port);
		bool Connect(const std::string &ip, uint port);
		bool Disconnect();
		bool Listen(uint max_conections);
		bool HasPendingConnection(bool &pending);
		bool HasPendingData(uint &size);
		SocketImpl *Accept();
		SocketImpl *Accept(const std::string &out_addr, uint &out_port);
		bool SendTo(const byte *data, uint count, uint &bytes_sent, const std::string &dest_ip, uint dest_port);
		bool Send(const byte *data, uint count, uint &bytes_sent);
		bool ReceiveFrom(const byte *data, uint size, uint &bytes_read, const std::string &source_ip, uint &source_port, enums::ReceiveFlag flags);
		bool Receive(const byte *data, uint size, uint &bytes_read, enums::ReceiveFlag flags);
		bool Wait(enums::WaitCondition condition, uint wait_time);
		enums::ConnectionState GetConnectionState();
		bool GetPeerAddress(std::string &out_ip, uint &out_port);
		bool SetNonBlocking(bool non_blocking = true);
		bool SetBroadcast(bool allow_broadcast = true);
		bool JoinMulticastGroup(const std::string &ip, uint port);
		bool LeaveMulticastGroup(const std::string &ip, uint port);
		bool SetMulticastLoopback(bool loopback);
		bool SetMulticastTtl(unsigned short time_to_live);

	private:
		inline Win32Socket(enums::Type type, SOCKET new_socket) : SocketImpl(type)
		{
			socket_ptr = new_socket;
		}

		SOCKET socket_ptr = INVALID_SOCKET;
		struct addrinfo *result = nullptr, hints;
	private:
		bool SetupSocket(const std::string &ip, uint port);
	};
}