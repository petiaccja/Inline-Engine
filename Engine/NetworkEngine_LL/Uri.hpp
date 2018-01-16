// https://github.com/mfichman/http

#pragma once

#include <string>

namespace inl::net::http
{
	class Authority 
	{
	public:
		Authority(std::string const& user, std::string const& host, uint16_t port);
		Authority();

		std::string const& user() const 
		{ 
			return user_; 
		}

		std::string const& host() const 
		{ 
			return host_; 
		}

		uint16_t port() const 
		{ 
			return port_; 
		}

		void userIs(std::string const& user);
		void hostIs(std::string const& host);
		void portIs(uint16_t port);
	private:
		std::string user_;
		std::string host_;
		uint16_t port_;
	};

	class Uri {
	public:
		Uri(char* const value);
		Uri(std::string const& value);
		Uri();

		std::string const& scheme() const 
		{ 
			return scheme_;
		}

		Authority const& authority() const 
		{ 
			return authority_; 
		}

		std::string const& path() const 
		{ 
			return path_; 
		}

		std::string const& host() const 
		{
			return authority_.host(); 
		}

		uint16_t port() const 
		{ 
			return authority_.port(); 
		}

		void schemeIs(std::string const& scheme);
		void authorityIs(Authority const& authority);
		void pathIs(std::string const& path);
	private:
		std::string scheme_;
		Authority authority_;
		std::string path_;
	};
}