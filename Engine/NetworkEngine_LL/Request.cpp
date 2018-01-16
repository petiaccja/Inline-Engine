// https://github.com/mfichman/http

#include "Request.hpp"

namespace inl::net::http
{
	std::string const Request::header(std::string const& name) const 
	{
		return headers_.header(name);
	}

	void Request::methodIs(Method method) 
	{
		method_ = method;
	}

	void Request::uriIs(Uri const& uri) 
	{
		uri_ = uri;
	}

	void Request::dataIs(std::string const& data) 
	{
		data_ = data;
	}

	void Request::headerIs(std::string const& name, std::string const& value) 
	{
		headers_.headerIs(name, value);
	}
}