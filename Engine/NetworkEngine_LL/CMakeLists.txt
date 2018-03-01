# NETWORKAPI

# Files
set(util
	"Util.hpp"
)

set(http
	"Cookies.cpp"
	"Cookies.hpp"
	"Headers.cpp"
	"Headers.hpp"
	"Http.cpp"
	"Http.hpp"
	"Parse.hpp"
	"Request.cpp"
	"Request.hpp"
	"Response.cpp"
	"Response.hpp"
	"Uri.cpp"
	"Uri.hpp"
)

set(interfaces
	"Enums.hpp"
	"Init.hpp"
	"ISocket.hpp"
	"Net.hpp"
)

set(sockets
	"IPAddress.cpp"
	"IPAddress.hpp"
	"SecureSocket.cpp"
	"SecureSocket.hpp"
	"Socket.cpp"
	"Socket.hpp"
)

set(tcp
	"TcpClient.cpp"
	"TcpClient.hpp"
	"TcpListener.cpp"
	"TcpListener.hpp"
	"TcpSocketBuilder.cpp"
	"TcpSocketBuilder.hpp"
)

set(udp
	"UdpSocket.cpp"
	"UdpSocket.hpp"
	"UdpSocketBuilder.hpp"
)

# Target
add_library(NetworkApi STATIC
	${util}
	${http}
	${interfaces}
	${sockets}
	${tcp}
	${udp}
)


# Filters
source_group("Utility" FILES ${util})
source_group("Http" FILES ${http})
source_group("Interfaces" FILES ${interfaces})
source_group("Socket" FILES ${sockets})
source_group("Socket\\Tcp" FILES ${tcp})
source_group("Socket\\Udp" FILES ${udp})


# Dependencies
target_link_libraries(NetworkApi
	?<IF:${TARGET_PLATFORM_WINDOWS},ws2_32,>
)
