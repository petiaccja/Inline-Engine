#pragma once

#ifdef _MSC_VER

	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <windows.h>

	namespace inl::net::win32
	{
		static WSADATA WsaData;
		static bool Initialized;

		inline static bool Initialize()
		{
			return (Initialized = WSAStartup(MAKEWORD(2, 2), &WsaData)) != 0;
		}

		inline static void Cleanup()
		{
			if (Initialized)
			{
				WSACleanup();
				Initialized = false;
			}
		}
	}

#else

	namespace inl::net::win32
	{
		static bool Initialized = true;

		inline static bool Initialize() { return true; }
		inline static void Cleanup() { }
	}

#endif