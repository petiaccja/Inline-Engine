// https://github.com/mfichman/http

#include "Cookies.hpp"
#include "Parse.hpp"

#include <cassert>

namespace inl::net::http
{
	ParseResult<std::string> ParseName(const char* str) 
	{
		return ParseUntil(str, [](char ch) 
		{ 
			return isspace(ch) || ch == '='; 
		});
	}

	ParseResult<std::string> ParseValue(const char* str) 
	{
		return ParseUntil(str, [](char ch) 
		{ 
			return ch == ';' || ch == '='; 
		});
	}

	ParseResult<std::string> ParseSeparator(const char* str) 
	{
		if (*str) 
		{
			assert(*str == ';' || *str == '=');
			return ParseWhitespace(str + 1);
		}
		else 
		{
			auto result = ParseResult<std::string>{};
			result.ch = str;
			return result;
		}
	}

	Cookie ParseCookie(const char* str) 
	{
		auto name = ParseName(str);
		auto ch = ParseSeparator(name.ch).ch;
		auto value = ParseValue(ch);
		ch = ParseSeparator(value.ch).ch;

		auto cookie = Cookie();
		cookie.SetName(name.value);
		cookie.SetValue(value.value);
		while (*ch) 
		{
			auto flag = ParseValue(ch);
			if (flag.value == "Path") 
			{
				ch = ParseSeparator(flag.ch).ch;
				flag = ParseValue(ch);
				cookie.SetPath(flag.value);
			}
			else if (flag.value == "HttpOnly")
				cookie.SetHttpOnly(true);
			else if (flag.value == "Secure")
				cookie.SetSecure(true);
			ch = ParseSeparator(flag.ch).ch;
		}
		return cookie;
	}

	Cookie::Cookie(const std::string& text) 
	{
		*this = ParseCookie(text.c_str());
	}

	const Cookie Cookies::operator[](const std::string & name) const
	{
		auto i = m_cookie.find(name);
		return (i == m_cookie.end()) ? Cookie() : i->second;
	}

	void Cookies::SetCookie(const Cookie& cookie)
	{
		m_cookie[cookie.GetName()] = cookie;
	}
}