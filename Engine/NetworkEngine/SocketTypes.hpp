#pragma once


namespace inl::net::enums
{
	enum class Type
	{
		Unknown,
		TCP,
		UDP
	};

	enum class IPVersion
	{
		IPv4,
		IPv6
	};

	enum class ConnectionState
	{
		Connected,
		Disconnected
	};

	enum class ReceiveFlag
	{
		None,
		Peek = 2,
		WaitAll = 0x100
	};

	enum class WaitCondition
	{
		WaitForRead,
		WaitForWrite,
		WaitForReadOrWrite
	};
}