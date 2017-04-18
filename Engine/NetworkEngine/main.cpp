#include "Win32Socket.hpp"
#include "Init.hpp"
#include <iostream>
#include "Util.hpp"

using namespace inl::net;

int wmain()
{
	win32::Win32Init::Initialize();

	Win32Socket *socket = new Win32Socket(enums::Type::TCP);

	socket->Bind(5005);
	socket->Listen(500);

	while (true)
	{
		/*if (socket->HasPendingConnection())
		{*/
			Win32Socket *soc = (Win32Socket*)socket->Accept();

			if (soc)
			{
				std::cout << "Accepted" << std::endl;

				/*uint size;
				uint read_size;
				char *arr = nullptr;
				while (soc->isConnected)
				{
					size = 0;
					read_size = 0;
					Delete(arr);
					while (soc->HasPendingData(size)) 
					{
						arr = new char[size + 1](); // when creating a new array the size must be the pending data size +1 to null-terminate the string
						uint temp_read_size = 0;
						soc->Receive(arr, size, temp_read_size);
						read_size += temp_read_size;
					}

					std::cout << arr << std::endl;
				}*/

				/*char *message = "Hello world\0";
				uint sent;
				soc->Send(message, strlen(message), sent);*/ 
			}
			else
				delete soc;
		//}
	}

	win32::Win32Init::Cleanup();
	return 0;
}