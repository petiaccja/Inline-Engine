#pragma once

#include <string>
#include "Types.hpp"
#include "SocketTypes.hpp"

namespace inl::net
{
	struct SocketImpl
	{
	protected:
		enums::Type socket_type = enums::Type::Unknown;
		enums::IPVersion ip_version = enums::IPVersion::IPv4;

	public:
		inline SocketImpl() { }
		inline SocketImpl(enums::Type type) : socket_type(type) { }
		virtual ~SocketImpl() { }

		virtual void Close() = 0;
		virtual bool Bind(uint port) = 0;
		virtual bool Connect(const std::string &ip, uint port) = 0;
		virtual bool Disconnect() = 0;
		virtual bool Listen(uint max_conections = 0) = 0; // if max_connections == 0 { max_connections = infinite }
		virtual bool HasPendingConnection() = 0;
		virtual bool HasPendingData(uint &size) = 0;
		virtual SocketImpl *Accept() = 0;
		virtual bool SendTo(const char *data, uint count, uint &bytes_sent, const std::string &dest_ip, uint dest_port) = 0;
		virtual bool Send(const char *data, uint count, uint &bytes_sent) = 0;
		virtual bool ReceiveFrom(const char *data, uint size, uint &bytes_read, const std::string &source_ip, uint &source_port, enums::ReceiveFlag flags = enums::ReceiveFlag::None) = 0;
		virtual bool Receive(const char *data, uint size, uint &bytes_read, enums::ReceiveFlag flags = enums::ReceiveFlag::None) = 0;
		virtual bool Wait(enums::WaitCondition condition, uint wait_time) = 0;
		virtual enums::ConnectionState GetConnectionState() = 0;
		virtual bool GetPeerAddress(std::string &out_ip, uint &out_port) = 0;
		virtual bool SetNonBlocking(bool non_blocking = true) = 0;
		virtual bool SetBroadcast(bool allow_broadcast = true) = 0;
		virtual bool JoinMulticastGroup(const std::string &ip, uint port) = 0;
		virtual bool LeaveMulticastGroup(const std::string &ip, uint port) = 0;
		virtual bool SetMulticastLoopback(bool loopback) = 0;
		virtual bool SetMulticastTtl(unsigned short time_to_live) = 0;
		
		inline void SetIPVersion(enums::IPVersion version)
		{
			ip_version = version;
		}

		inline void SetSendBufferSize(int size)
		{
			send_buffer_len = size;
		}

		inline void SetReceiveBufferSize(int size)
		{
			receive_buffer_len = size;
		}

		inline void SetBufferSize(int size)
		{
			SetSendBufferSize(size);
			SetReceiveBufferSize(size);
		}

		inline enums::Type GetSocketType() const
		{
			return socket_type;
		}

		inline enums::IPVersion GetIPVersion() const
		{
			return ip_version;
		}

		bool isConnected;
		std::string IPAddress;
		uint Port;
	private:
		uint send_buffer_len = 1024, receive_buffer_len = 1024;
	};
}