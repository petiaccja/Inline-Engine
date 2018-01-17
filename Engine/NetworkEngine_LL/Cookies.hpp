// https://github.com/mfichman/http

#pragma once

#include <map>

namespace inl::net::http
{
	class Cookie 
	{
	public:
		Cookie(const std::string& text);

		Cookie() : m_httpOnly(false), m_secure(false)
		{
		}

		const std::string& GetName() const 
		{ 
			return m_name;
		}

		const std::string& GetValue() const 
		{ 
			return m_value;
		}

		const std::string& GetPath() const 
		{ 
			return m_path;
		}

		bool IsHttpOnly() const 
		{ 
			return m_httpOnly;
		}

		bool IsSecure() const 
		{ 
			return m_secure;
		}

		void SetName(const std::string& name) 
		{ 
			m_name = name;
		}

		void SetValue(const std::string& value) 
		{ 
			m_value = value;
		}

		void SetPath(const std::string& path) 
		{ 
			m_path = path;
		}

		void SetHttpOnly(bool httpOnly) 
		{ 
			m_httpOnly = httpOnly;
		}

		void SetSecure(bool secure) 
		{ 
			m_secure = secure;
		}

	private:
		std::string m_name;
		std::string m_value;
		std::string m_path;
		bool m_httpOnly;
		bool m_secure;
	};

	class Cookies 
	{
	public:
		const Cookie operator[](const std::string &name) const;

		std::map<std::string, Cookie>::const_iterator begin() const
		{ 
			return m_cookie.begin();
		}

		std::map<std::string, Cookie>::const_iterator end() const
		{ 
			return m_cookie.end();
		}

		void SetCookie(Cookie const& cookie);

		static const std::string HOST;
		static const std::string CONTENT_LENGTH;
		static const std::string ACCEPT_ENCODING;
		static const std::string CONNECTION;
	private:
		std::map<std::string, Cookie> m_cookie;
	};
}