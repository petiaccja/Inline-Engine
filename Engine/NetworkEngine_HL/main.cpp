#include "Init.hpp"
#include "Server.hpp"

int main()
{
	inl::net::Initialize();
	inl::net::Server server(100);
	server.Start();
	while (true)
	{

	}
}