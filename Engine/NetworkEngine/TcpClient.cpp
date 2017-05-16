#include "TcpClient.hpp"
#include "Util.hpp"
#include "Init.hpp"

namespace inl::net::tcp
{
	TcpClient::TcpClient(const std::string &ip, int port) :
		ip(ip), port(port)
	{
		initialize(ip, port);
	}

	TcpClient::TcpClient(const SOCKET & socket)
	{
		this->soc = socket;
	}

	TcpClient::~TcpClient()
	{
		//disconnect
		freeaddrinfo(result);
		WSACleanup();
		util::Delete(result);
	}

	bool TcpClient::DataAvailable(int & size)
	{
		return ioctlsocket(soc, FIONREAD, reinterpret_cast<u_long*>(size)) != NO_ERROR && size > 0;
	}

	bool TcpClient::initialize(const std::string &ip, int port)
	{
		if (!win32::Initialized)
			win32::Initialize();
		if (util::ValidIPv4Addr(ip) || port == 0)
			return false;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		if (getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result) != 0)
		{
			WSACleanup();
			return false;
		}

		if ((soc = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET)
		{
			freeaddrinfo(result);
			WSACleanup();
			return false;
		}

		return true;
	}

	NetworkBuffer TcpClient::receive_buffer()
	{
		NetworkBuffer buffer;

		int temp;
		if (DataAvailable(temp) && temp > sizeof(int))
		{
			char *header = new char[sizeof(int)]();
			if (recv(soc, reinterpret_cast<char*>(header), sizeof(int), 0) != sizeof(int))
				return NetworkBuffer();
			try
			{
				buffer.BodySize = std::stoi(header);
			}
			catch (std::exception)
			{
				// header var doesnt contain int
			}
		}
		else
			return NetworkBuffer();

		buffer.Body = new char[buffer.BodySize]();
		int received_bytes = recv(soc, reinterpret_cast<char*>(buffer.Body), buffer.BodySize, 0);
		if (received_bytes == SOCKET_ERROR || received_bytes != buffer.BodySize || WSAGetLastError() != 0)
			return NetworkBuffer();
		buffer.Valid = true; // we presume its valid
		return buffer;
	}
}