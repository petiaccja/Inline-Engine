// https://github.com/mfichman/http

#include "Headers.hpp"

namespace inl::net::http
{
	std::string const Headers::HOST("Host");
	std::string const Headers::CONTENT_LENGTH("Content-Length");
	std::string const Headers::ACCEPT_ENCODING("Accept-Encoding");
	std::string const Headers::CONNECTION("Connection");

	std::string const Headers::header(std::string const& name) const 
	{
		auto i = header_.find(name);
		return (i == header_.end()) ? "" : i->second;
	}

	void Headers::headerIs(std::string const& name, std::string const& value) 
	{
		header_.emplace(name, value);
	}
}