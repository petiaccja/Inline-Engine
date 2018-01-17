// https://github.com/mfichman/http

#include "Response.hpp"
#include "Parse.hpp"

#include "BaseLibrary/Exception/Exception.hpp"

namespace inl::net::http
{
	static ParseResult<HttpStatus> ParseStatus(const char* str)
	{
		ParseResult<HttpStatus> result
		{
		};

		auto code = ParseToken(str);

		result.value = (HttpStatus)std::atoi(code.value.c_str());
		result.ch = code.ch;
		return result;
	}

	Response ParseResponse(const char* str) 
	{
		// Parse an HTTP response 
		auto version = ParseToken(str);
		auto code = ParseStatus(version.ch);
		auto message = ParseUntil(code.ch, [](char ch) 
		{ 
			return ch == '\r'; 
		});

		auto response = Response();
		if (version.value != "HTTP/1.1")
			throw inl::RuntimeException("bad HTTP version");

		auto ch = parseCrLf(message.ch).ch;
		while (*ch != '\0' && *ch != '\r') 
		{
			auto name = ParseUntil(ch, [](char ch) 
			{ 
				return ch == ':'; 
			});

			if (*name.ch)
				name.ch++; // For ":"
			auto ws = ParseWhile(name.ch, isspace);
			auto value = ParseUntil(ws.ch, [](char ch) 
			{ 
				return ch == '\r'; 
			});

			response.SetHeader(name.value, value.value);
			if (name.value == "Set-Cookie")
				response.SetCookie(Cookie(value.value));
			ch = parseCrLf(value.ch).ch;
		}
		ch = parseCrLf(ch).ch;

		response.SetStatus(code.value);
		response.SetData(ch);
		return response;
	}

	Response::Response(const std::string& response) 
	{
		*this = ParseResponse(response.c_str());
	}

	const std::string Response::GetHeader(const std::string& name) const 
	{
		return m_headers[name];
	}

	const Cookie Response::GetCookie(const std::string& name) const 
	{
		return m_cookies[name];
	}

	void Response::SetStatus(HttpStatus status)
	{
		m_status = status;
	}

	void Response::SetData(const std::string& data) 
	{
		m_data = data;
	}

	void Response::SetHeader(const std::string& name, const std::string& value) 
	{
		m_headers.AddHeader(name, value);
	}

	void Response::SetCookie(const Cookie& cookie) 
	{
		m_cookies.SetCookie(cookie);
	}
}