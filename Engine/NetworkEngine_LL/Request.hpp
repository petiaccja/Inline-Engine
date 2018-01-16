// https://github.com/mfichman/http

#pragma once

#include "Uri.hpp"
#include "Headers.hpp"

namespace inl::net::http
{
	class Request 
	{
	public:
		enum Method { GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT };

		Method method() const 
		{ 
			return method_; 
		}

		Uri const& uri() const 
		{ 
			return uri_; 
		}

		std::string const& path() const 
		{ 
			return uri_.path();
		}

		std::string const& data() const 
		{ 
			return data_; 
		}

		std::string const header(std::string const& name) const;

		Headers const& headers() const 
		{ 
			return headers_; 
		}

		void methodIs(Method method);
		void uriIs(Uri const& path);
		void dataIs(std::string const& data);
		void headerIs(std::string const& name, std::string const& value);

	private:
		Method method_ = GET;
		Uri uri_;
		std::string data_;
		Headers headers_;
	};
}