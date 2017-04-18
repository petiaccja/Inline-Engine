#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

namespace inl::net::win32
{
	WSADATA WsaData;
	bool Initialized;

	class Win32Init
	{
	public:
		static inline bool Initialize()
		{
			return (Initialized = WSAStartup(MAKEWORD(2, 2), &WsaData)) != 0;
		}

		static inline void Cleanup()
		{
			if (Initialized)
			{
				WSACleanup();
				Initialized = false;
			}
		}
	};
}