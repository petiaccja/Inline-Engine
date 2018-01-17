// https://github.com/mfichman/http

#pragma once

#include <map>

namespace inl::net::http
{
	class Headers 
	{
	public:
		const std::string operator[](const std::string &name) const;

		std::multimap<std::string, std::string>::const_iterator begin() const
		{ 
			return m_header.begin();
		}

		std::multimap<std::string, std::string>::const_iterator end() const
		{ 
			return m_header.end();
		}

		void AddHeader(std::string const& name, std::string const& value);

		static std::string const HOST;
		static std::string const CONTENT_LENGTH;
		static std::string const ACCEPT_ENCODING;
		static std::string const CONNECTION;
	private:
		std::multimap<std::string, std::string> m_header;
};
}