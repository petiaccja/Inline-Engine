// https://github.com/mfichman/http

#include "Headers.hpp"

namespace inl::net::http
{
	std::string const Headers::HOST("Host");
	std::string const Headers::CONTENT_LENGTH("Content-Length");
	std::string const Headers::ACCEPT_ENCODING("Accept-Encoding");
	std::string const Headers::CONNECTION("Connection");

	const std::string Headers::operator[](const std::string & name) const
	{
		auto i = m_header.find(name);
		return (i == m_header.end()) ? "" : i->second;
	}

	void Headers::AddHeader(std::string const& name, std::string const& value) 
	{
		m_header.emplace(name, value);
	}
}