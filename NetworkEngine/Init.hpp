#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

namespace inl::net::win32
{
	class Win32Init
	{
	public:
		inline bool Initialize()
		{
			return initialized = WSAStartup(MAKEWORD(2, 2), &data) != 0;
		}

		inline void Cleanup()
		{
			if (initialized)
			{
				WSACleanup();
				initialized = false;
			}
		}
	private:
		bool initialized;
		WSADATA data;
	};
}