// https://github.com/mfichman/http

#pragma once

#include <string>

namespace inl::net::http
{
	template <typename T>
	class ParseResult 
	{
	public:
		T value;
		char const* ch;
	};

	template <typename F>
	static inline ParseResult<std::string> parseUntil(char const* str, F func) 
	{
		ParseResult<std::string> result{};
		char const* ch = str;
		for (; *ch && !func(*ch); ++ch) 
		{
		}

		result.value = std::string(str, ch - str);
		result.ch = ch;
		return result;
	}

	template <typename F>
	static inline ParseResult<std::string> parseWhile(char const* str, F func) 
	{
		ParseResult<std::string> result{};
		char const* ch = str;
		for (; *ch && func(*ch); ++ch) 
		{
		}

		result.value = std::string(str, ch - str);
		result.ch = ch;
		return result;
	}

	static inline ParseResult<std::string> parseToken(char const* str) 
	{
		auto token = parseUntil(str, isspace);
		token.ch = parseWhile(token.ch, isspace).ch;
		return token;
	}

	static inline ParseResult<std::string> parseCrLf(char const* str) 
	{
		auto cr = parseUntil(str, [](char ch) { return ch == '\r'; });
		if (*cr.ch == '\r')
			cr.ch++;
		return parseWhile(cr.ch, [](char ch) 
		{ 
			return isspace(ch) && ch != '\r'; 
		});
	}

	static inline ParseResult<std::string> parseWhitespace(char const* str) 
	{
		return parseWhile(str, isspace);
	}
}