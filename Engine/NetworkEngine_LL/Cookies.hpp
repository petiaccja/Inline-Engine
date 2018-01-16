// https://github.com/mfichman/http

#pragma once

#include <map>

namespace inl::net::http
{
	class Cookie 
	{
	public:
		Cookie(std::string const& text);

		Cookie() : httpOnly_(false), secure_(false) 
		{
		}

		std::string const& name() const 
		{ 
			return name_; 
		}

		std::string const& value() const 
		{ 
			return value_; 
		}

		std::string const& path() const 
		{ 
			return path_; 
		}

		bool httpOnly() const 
		{ 
			return httpOnly_; 
		}

		bool secure() const 
		{ 
			return secure_; 
		}

		void nameIs(std::string const& name) 
		{ 
			name_ = name; 
		}

		void valueIs(std::string const& value) 
		{ 
			value_ = value; 
		}

		void pathIs(std::string const& path) 
		{ 
			path_ = path; 
		}

		void httpOnlyIs(bool httpOnly) 
		{ 
			httpOnly_ = httpOnly; 
		}

		void secureIs(bool secure) 
		{ 
			secure_ = secure; 
		}

	private:
		std::string name_;
		std::string value_;
		std::string path_;
		bool httpOnly_;
		bool secure_;
	};

	class Cookies {
	public:
		typedef std::map<std::string, Cookie> Map;

		Cookie const cookie(std::string const& name) const;

		Map::const_iterator begin() const 
		{ 
			return cookie_.begin(); 
		}

		Map::const_iterator end() const 
		{ 
			return cookie_.end(); 
		}

		void cookieIs(Cookie const& cookie);

		static std::string const HOST;
		static std::string const CONTENT_LENGTH;
		static std::string const ACCEPT_ENCODING;
		static std::string const CONNECTION;
	private:
		Map cookie_;
	};
}