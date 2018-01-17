// https://github.com/mfichman/http

#include "Http.hpp"

#include <cassert>
#include <vector>
#include <sstream>

#include "Socket.hpp"
#include "SecureSocket.hpp"

#undef DELETE

namespace inl::net::http
{
	using namespace sockets;

	Response Http::Send(Request const& request) 
	{
		// Send an HTTP request.  Auto-fill the content-length headers.
		std::string string = Str(request);
		
		uint16_t port = 0;
		std::unique_ptr<Socket> socket;
		std::unique_ptr<SecureSocket> secure_socket;
		bool secure = false;
		if (request.GetUri().GetScheme() == "https") 
		{
			secure_socket.reset(new SecureSocket());
			port = 443;
			secure = true;
		}
		else if (request.GetUri().GetScheme() == "http") 
		{
			socket.reset(new Socket(SocketType::Streaming));
			port = 80;
		}
		else
			assert(!"unknown http scheme");

		if (request.GetUri().GetPort())
			port = request.GetUri().GetPort();

		secure ? secure_socket->Connect(IPAddress(request.GetUri().GetHost(), port)) : socket->Connect(IPAddress(request.GetUri().GetHost(), port));
		int32_t sent;
		secure ? secure_socket->Recv((uint8_t*)string.c_str(), string.size(), sent) : socket->Send((uint8_t*)string.c_str(), string.size(), sent);

		std::vector<char> buffer(16384); // 16 KiB
		std::stringstream ss;

		int32_t read;
		do 
		{
			secure ? secure_socket->Recv((uint8_t*)&buffer[0], buffer.size(), read) : socket->Recv((uint8_t*)&buffer[0], buffer.size(), read);
			ss.write(&buffer[0], read);
		} 
		while (read > 0);
		secure ? secure_socket->Close() : socket->Close();

		return Response(ss.str());
	}

	Response Http::Get(std::string const& path, std::string const& data) 
	{
		// Shortcut for simple GET requests
		Request request;
		request.SetMethod(Method::GET);
		request.SetUri(Uri(path));
		request.SetData(data);
		return Send(request);
	}

	Response Http::Post(std::string const& path, std::string const& data) 
	{
		// Shortcut for simple POST requests
		Request request;
		request.SetMethod(Method::POST);
		request.SetUri(Uri(path));
		request.SetData(data);
		return Send(request);
	}

	static std::string str_impl(Method method) {
		switch (method) 
		{
			case Method::GET:
				return "GET";
			case Method::HEAD:
				return "HEAD";
			case Method::POST:
				return "POST";
			case Method::PUT:
				return "PUT";
			case Method::DELETE:
				return "DELETE";
			case Method::TRACE:
				return "TRACE";
			case Method::CONNECT:
				return "CONNECT";
			default: 
				assert(!"unknown request method");
		}
		return "";
	}

	std::string Http::Str(Request const& request) 
	{
		// Serialize a request to a string
		std::stringstream ss;
		auto path = request.GetPath().empty() ? "/" : request.GetPath();
		ss << str_impl(request.GetMethod()) << ' ' << path << " HTTP/1.1\n";
		ss << Headers::HOST << ": " << request.GetUri().GetHost() << "\n";
		ss << Headers::CONTENT_LENGTH << ": " << request.GetData().size() << "\n";
		ss << Headers::CONNECTION << ": close\n";
		ss << Headers::ACCEPT_ENCODING << ": identity\n";
		for (auto header : request.GetHeaders())
			ss << header.first << ": " << header.second << "\n";
		ss << "\n";
		ss << request.GetData();
		return ss.str();
	}
}