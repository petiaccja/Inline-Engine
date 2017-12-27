#pragma once

#include <stdint.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#undef SendMessage
#undef SetPort

#else
	#include <unistd.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/ioctl.h>

	#define SOCKET_ERROR -1
	#define NO_ERROR 0
	#define INVALID_SOCKET NO_ERROR 

	#define SOCKET int
	#define closesocket close
	#define ioctlsocket ioctl

#endif

#define DEFAULT_SERVER_PORT 61250
