# NETWORKENGINE

# Files
set(common
	"InternalTags.hpp"
	"NetworkHeader.hpp"
	"NetworkMessage.cpp"
	"NetworkMessage.hpp"
	"TcpConnection.cpp"
	"TcpConnection.hpp"
)

set(events
	"DataReceivedEvent.hpp"
	"DisconnectedEvent.hpp"
	"NewConnectionEvent.hpp"
)

set(tcp
	"TcpConnectionHandler.cpp"
	"TcpConnectionHandler.hpp"
	"TcpServer.cpp"
	"TcpServer.hpp"
)

set(global
	"BitConverter.hpp"
	"main.cpp"
	"MessageQueue.cpp"
	"MessageQueue.hpp"
	"Server.cpp"
	"Server.hpp"
)


# Target
add_library(NetworkEngine STATIC
	${common}
	${events}
	${tcp}
	${global}
)


# Filters
source_group("Common" FILES ${common})
source_group("Events" FILES ${events})
source_group("Tcp" FILES ${tcp})
source_group("" FILES ${global})


# Dependencies
target_link_libraries(NetworkEngine
	NetworkApi
)
