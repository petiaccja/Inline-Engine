// https://github.com/mfichman/http

#include "Response.hpp"
#include "Parse.hpp"

#include "BaseLibrary/Exception/Exception.hpp"

namespace inl::net::http
{
	static ParseResult<Response::Status> parseStatus(char const* str) 
	{
		ParseResult<Response::Status> result
		{
		};

		auto code = parseToken(str);

		result.value = (Response::Status)std::atoi(code.value.c_str());
		result.ch = code.ch;
		return result;
	}

	Response parseResponse(char const* str) 
	{
		// Parse an HTTP response 
		auto version = parseToken(str);
		auto code = parseStatus(version.ch);
		auto message = parseUntil(code.ch, [](char ch) 
		{ 
			return ch == '\r'; 
		});

		auto response = Response();
		if (version.value != "HTTP/1.1")
			throw inl::RuntimeException("bad HTTP version");

		auto ch = parseCrLf(message.ch).ch;
		while (*ch != '\0' && *ch != '\r') 
		{
			auto name = parseUntil(ch, [](char ch) 
			{ 
				return ch == ':'; 
			});

			if (*name.ch)
				name.ch++; // For ":"
			auto ws = parseWhile(name.ch, isspace);
			auto value = parseUntil(ws.ch, [](char ch) 
			{ 
				return ch == '\r'; 
			});

			response.headerIs(name.value, value.value);
			if (name.value == "Set-Cookie")
				response.cookieIs(Cookie(value.value));
			ch = parseCrLf(value.ch).ch;
		}
		ch = parseCrLf(ch).ch;

		response.statusIs(code.value);
		response.dataIs(ch);
		return response;
	}

	Response::Response(std::string const& response) 
	{
		*this = parseResponse(response.c_str());
	}

	std::string const Response::header(std::string const& name) const 
	{
		return headers_.header(name);
	}

	Cookie const Response::cookie(std::string const& name) const 
	{
		return cookies_.cookie(name);
	}

	void Response::statusIs(Status status) 
	{
		status_ = status;
	}

	void Response::dataIs(std::string const& data) 
	{
		data_ = data;
	}

	void Response::headerIs(std::string const& name, std::string const& value) 
	{
		headers_.headerIs(name, value);
	}

	void Response::cookieIs(Cookie const& cookie) 
	{
		cookies_.cookieIs(cookie);
	}
}