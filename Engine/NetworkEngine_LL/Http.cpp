// https://github.com/mfichman/http

#include "Http.hpp"

#include <cassert>
#include <vector>
#include <sstream>

#include "Socket.hpp"

#undef DELETE

namespace inl::net::http
{
	using namespace sockets;

	Response Http::send(Request const& request) 
	{
		// Send an HTTP request.  Auto-fill the content-length headers.
		std::string string = str(request);

		uint16_t port = 0;
		std::unique_ptr<Socket> socket;
		if (request.uri().scheme() == "https") 
		{
			//socket.reset(new SslSocket(SocketType::Streaming));
			//port = 443;
		}
		else if (request.uri().scheme() == "http") 
		{
			socket.reset(new Socket(SocketType::Streaming));
			port = 80;
		}
		else
			assert(!"unknown http scheme");

		if (request.uri().port())
			port = request.uri().port();

		socket->Connect(IPAddress(request.uri().host(), port));
		int32_t sent;
		socket->Send((uint8_t*)string.c_str(), string.size(), sent);

		std::vector<char> buffer(16384); // 16 KiB
		std::stringstream ss;

		int32_t read;
		do 
		{
			socket->Recv((uint8_t*)&buffer[0], buffer.size(), read);
			ss.write(&buffer[0], read);
		} 
		while (read > 0);

		return Response(ss.str());
	}

	Response Http::get(std::string const& path, std::string const& data) 
	{
		// Shortcut for simple GET requests
		Request request;
		request.methodIs(Request::GET);
		request.uriIs(Uri(path));
		request.dataIs(data);
		return send(request);
	}

	Response Http::post(std::string const& path, std::string const& data) 
	{
		// Shortcut for simple POST requests
		Request request;
		request.methodIs(Request::POST);
		request.uriIs(Uri(path));
		request.dataIs(data);
		return send(request);
	}

	static std::string str_impl(Request::Method method) {
		switch (method) 
		{
			case Request::GET: 
				return "GET";
			case Request::HEAD: 
				return "HEAD";
			case Request::POST:
				return "POST";
			case Request::PUT: 
				return "PUT";
			case Request::DELETE: 
				return "DELETE";
			case Request::TRACE: 
				return "TRACE";
			case Request::CONNECT: 
				return "CONNECT";
			default: 
				assert(!"unknown request method");
		}
		return "";
	}

	std::string Http::str(Request const& request) 
	{
		// Serialize a request to a string
		std::stringstream ss;
		auto path = request.path().empty() ? "/" : request.path();
		ss << str_impl(request.method()) << ' ' << path << " HTTP/1.1\n";
		ss << Headers::HOST << ": " << request.uri().host() << "\n";
		ss << Headers::CONTENT_LENGTH << ": " << request.data().size() << "\n";
		ss << Headers::CONNECTION << ": close\n";
		ss << Headers::ACCEPT_ENCODING << ": identity\n";
		for (auto header : request.headers())
			ss << header.first << ": " << header.second << "\n";
		ss << "\n";
		ss << request.data();
		return ss.str();
	}
}