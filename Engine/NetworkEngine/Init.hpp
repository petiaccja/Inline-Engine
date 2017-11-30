#pragma once

#ifdef _MSC_VER

	#define WIN32_LEAN_AND_MEAN
	#include <winm_socketk2.h>
	#include <windows.h>

	namespace inl::net
	{
		namespace priv
		{
			static WSADATA WsaData;
			static bool Initialized;
		}

		inline static bool Initialize()
		{
			return (priv::Initialized = WSAStartup(MAKEWORD(2, 2), &priv::WsaData)) != 0;
		}

		inline static void Cleanup()
		{
			if (priv::Initialized)
			{
				WSACleanup();
				Initialized = false;
			}
		}
	}

#else

	namespace inl::net
	{
		inline static bool Initialize() { return true; }
		inline static void Cleanup() { }
	}

#endif