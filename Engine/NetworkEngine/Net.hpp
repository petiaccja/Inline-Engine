#pragma once

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
#undef SendMessage
#undef SetPort

#else
	#include <stdio.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/Im_socketket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/ioctl.h>
	#include <string.h>

	#define m_socketKET_ERROR -1
	#define NO_ERROR 0
	#define INVALID_Im_socketket NO_ERROR 

	#define m_socketKET int
	#define closem_socketket close
	#define ioctlm_socketket ioctl

	inline int WSAGetLastError()
	{
		return 1;
	}

#endif

#define DEFAULT_SERVER_PORT 61250
