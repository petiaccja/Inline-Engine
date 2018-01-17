// https://github.com/mfichman/http

#include "Uri.hpp"
#include "Parse.hpp"

namespace inl::net::http
{
	static bool IsReserved(char ch) 
	{
		switch (ch) 
		{
			case '/': 
				return true;
			case '#': 
				return true;
			case '?': 
				return true;
			default: 
				return false;
		}
	}

	static ParseResult<std::string> ParseScheme(char const* str) 
	{
		auto result = ParseWhile(str, [](char ch) 
		{
			return ch != ':' && !IsReserved(ch);
		});

		result.ch = (result.ch[0] == ':') ? (result.ch + 1) : (result.ch);
		return result;
	}

	static ParseResult<std::string> ParseUser(char const* str) 
	{
		auto result = ParseWhile(str, [](char ch) 
		{
			return ch != '@' && !IsReserved(ch);
		});

		if (result.ch[0] == '@')
			result.ch = result.ch + 1;
		else {
			result.ch = str;
			result.value = "";
		}
		return result;
	}

	static ParseResult<std::string> ParseHost(char const* str) 
	{
		return ParseWhile(str, [](char ch) 
		{
			return ch != ':' && !IsReserved(ch);
		});
	}

	static ParseResult<uint16_t> ParsePort(char const* str) 
	{
		ParseResult<uint16_t> result;
		if (str[0] != ':') 
		{
			result.value = 0;
			result.ch = str;
			return result;
		}

		auto tmp = ParseWhile(str + 1, [](char ch) 
		{
			return !IsReserved(ch);
		});

		result.value = uint16_t(strtol(tmp.value.c_str(), 0, 10));
		result.ch = tmp.ch;
		return result;
	}

	static ParseResult<Authority> parseAuthority(char const* str) 
	{
		ParseResult<Authority> result
		{
		};

		if (str[0] == '\0' || str[0] != '/' || str[1] != '/') 
		{
			result.ch = str;
			return result;
		}

		auto user = ParseUser(str + 2); // For "//"
		auto host = ParseHost(user.ch);
		auto port = ParsePort(host.ch);

		result.value.SetUser(user.value);
		result.value.SetHost(host.value);
		result.value.SetPort(port.value);
		result.ch = port.ch;

		return result;
	}

	static ParseResult<std::string> parsePath(char const* str) 
	{
		// Return query/frag as part of path for now
		ParseResult<std::string> result = ParseWhile(str, [](char ch) 
		{
			return true;
		});
		/*
		ParseResult<std::string> result = parseWhile(str, [](char ch) {
		return ch != '/' && !isReserved(ch);
		});
		result.ch = (result.ch[0] == '?') ? (result.ch+1) : (result.ch);
		*/
		return result;

	}

	static Uri parseUri(char const* str) 
	{
		Uri uri;

		auto scheme = ParseScheme(str);
		auto authority = parseAuthority(scheme.ch);
		auto path = parsePath(authority.ch);

		uri.SetScheme(scheme.value);
		uri.SetAuthority(authority.value);
		uri.SetPath(path.value);
		return uri;
	}


	Authority::Authority(std::string const& user, std::string const& host, uint16_t port) 
	{
		m_user = user;
		m_host = host;
		m_port = port;
	}

	Authority::Authority() 
	{
		m_port = 0;
	}

	void Authority::SetUser(std::string const& user) 
	{
		m_user = user;
	}

	void Authority::SetHost(std::string const& host) 
	{
		m_host = host;
	}

	void Authority::SetPort(uint16_t port) 
	{
		m_port = port;
	}

	Uri::Uri(const char* value) 
	{
		*this = parseUri(value);
	}

	Uri::Uri(const std::string& value) 
	{
		*this = parseUri(value.c_str());
	}

	Uri::Uri() {
	}

	void Uri::SetScheme(const std::string& scheme) 
	{
		m_scheme = scheme;
	}

	void Uri::SetAuthority(const Authority& authority)
	{
		m_authority = authority;
	}

	void Uri::SetPath(const std::string& path) 
	{
		m_path = path;
	}
}