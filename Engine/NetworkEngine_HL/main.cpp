#include "Init.hpp"
#include "Server.hpp"
#include "BitConverter.hpp"

#include <iostream>

int main()
{
	inl::net::Initialize();
	inl::net::Server server(100);
	server.Start();
	
	inl::net::sockets::TcpClient client;
	inl::net::IPAddress ip("127.0.0.1");
	client.Connect(ip);

	while (true)
	{
		uint32_t data_size;
		while (client.HasPendingData(data_size))
		{
			inl::net::NetworkMessage message;

			uint8_t *bytes = new uint8_t[data_size]();

			int32_t read;
			client.Recv(bytes, data_size, read);

			message.Deserialize(bytes, data_size);

			uint32_t id = inl::BitConverter::FromBytes<uint32_t>((uint8_t*)(message.GetData<void>()));
			std::cout << id << std::endl;
		}
	}
}