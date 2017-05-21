#pragma once

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
#undef SendMessage

#define DEFAULT_LISTEN_PORT 61250

#else
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

	#define SOCKET_ERROR -1
	#define INVALID_SOCKET 0

	typedef int SOCKET;

	inline void closesockets(SOCKET socket) 
	{ 
		//close(socket); 
	}

	inline char *itoa(int n, char *dummy_char, int dummy_radix)
	{
		char *str = new char[sizeof(int)]();
		snprintf(str, sizeof(int), "%d", n);
		return str;
	}

	inline int WSAGetLastError()
	{
		return 1;
	}

#endif