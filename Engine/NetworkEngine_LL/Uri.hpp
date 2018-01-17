// https://github.com/mfichman/http

#pragma once

#include <string>

namespace inl::net::http
{
	class Authority 
	{
	public:
		Authority(const std::string& user, const std::string& host, uint16_t port);
		Authority();

		const std::string& GetUser() const 
		{ 
			return m_user;
		}

		const std::string& GetHost() const 
		{ 
			return m_host;
		}

		uint16_t GetPort() const 
		{ 
			return m_port;
		}

		void SetUser(const std::string& user);
		void SetHost(const std::string& host);
		void SetPort(uint16_t port);
	private:
		std::string m_user;
		std::string m_host;
		uint16_t m_port;
	};

	class Uri {
	public:
		Uri(const char* value);
		Uri(const std::string& value);
		Uri();

		const std::string& GetScheme() const 
		{ 
			return m_scheme;
		}

		const Authority& GetAuthority() const 
		{ 
			return m_authority;
		}

		const std::string& GetPath() const 
		{ 
			return m_path;
		}

		const std::string& GetHost() const 
		{
			return m_authority.GetHost();
		}

		uint16_t GetPort() const 
		{ 
			return m_authority.GetPort();
		}

		void SetScheme(const std::string& scheme);
		void SetAuthority(const Authority& authority);
		void SetPath(const std::string& path);
	private:
		std::string m_scheme;
		Authority m_authority;
		std::string m_path;
	};
}