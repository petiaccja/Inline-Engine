// https://github.com/mfichman/http

#pragma once

#include <map>

namespace inl::net::http
{
	class Headers {
	public:
		typedef std::multimap<std::string,std::string> Map;

		std::string const header(std::string const& name) const;
		Map::const_iterator begin() const 
		{ 
			return header_.begin(); 
		}

		Map::const_iterator end() const 
		{ 
			return header_.end(); 
		}

		void headerIs(std::string const& name, std::string const& value);

		static std::string const HOST;
		static std::string const CONTENT_LENGTH;
		static std::string const ACCEPT_ENCODING;
		static std::string const CONNECTION;
	private:
		Map header_;
};
}