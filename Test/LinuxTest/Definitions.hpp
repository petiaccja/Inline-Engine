#pragma once

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
#undef SendMessage

#else
	#include <stdio.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>

	#define SOCKET_ERROR -1
	#define INVALID_SOCKET 0

	typedef int SOCKET;

	inline void closesocket(SOCKET socket) 
	{ 
		close(socket); 
	}

	inline int WSAGetLastError()
	{
		return 1;
	}

#endif

#define DEFAULT_LISTEN_PORT 61250
