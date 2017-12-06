#include "Init.hpp"
#include "Util.hpp"
#include "TcpListener.hpp"

#include <iostream>

int main()
{
	inl::net::Initialize();
	Socket s(SocketType::Streaming);
	IPAddress addr("127.0.0.1");
	s.Connect(addr);

	uint8_t *data = new uint8_t[100]();
	int32_t read;
	s.Recv(data, 100, read);

	std::cout << std::string((char*)data) << std::endl;

	getchar();
	return 0;
}