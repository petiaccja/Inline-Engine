// https://github.com/mfichman/http

#include "Cookies.hpp"
#include "Parse.hpp"

#include <cassert>

namespace inl::net::http
{
	ParseResult<std::string> parseName(char const* str) 
	{
		return parseUntil(str, [](char ch) 
		{ 
			return isspace(ch) || ch == '='; 
		});
	}

	ParseResult<std::string> parseValue(char const* str) 
	{
		return parseUntil(str, [](char ch) 
		{ return ch == ';' || ch == '='; 
		});
	}

	ParseResult<std::string> parseSeparator(char const* str) {
		if (*str) 
		{
			assert(*str == ';' || *str == '=');
			return parseWhitespace(str + 1);
		}
		else 
		{
			auto result = ParseResult<std::string>{};
			result.ch = str;
			return result;
		}
	}

	Cookie parseCookie(char const* str) 
	{
		auto name = parseName(str);
		auto ch = parseSeparator(name.ch).ch;
		auto value = parseValue(ch);
		ch = parseSeparator(value.ch).ch;

		auto cookie = Cookie();
		cookie.nameIs(name.value);
		cookie.valueIs(value.value);
		while (*ch) 
		{
			auto flag = parseValue(ch);
			if (flag.value == "Path") 
			{
				ch = parseSeparator(flag.ch).ch;
				flag = parseValue(ch);
				cookie.pathIs(flag.value);
			}
			else if (flag.value == "HttpOnly")
				cookie.httpOnlyIs(true);
			else if (flag.value == "Secure")
				cookie.secureIs(true);
			ch = parseSeparator(flag.ch).ch;
		}
		return cookie;
	}

	Cookie::Cookie(std::string const& text) 
	{
		*this = parseCookie(text.c_str());
	}

	Cookie const Cookies::cookie(std::string const& name) const 
	{
		auto i = cookie_.find(name);
		return (i == cookie_.end()) ? Cookie() : i->second;
	}

	void Cookies::cookieIs(Cookie const& cookie) 
	{
		cookie_[cookie.name()] = cookie;
	}
}