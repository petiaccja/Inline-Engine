#include "Util.hpp"
#include "TcpListener.hpp"

#include <iostream>

int main()
{
	TcpListener listener(DEFAULT_SERVER_PORT);
	getchar();
	return 0;
}