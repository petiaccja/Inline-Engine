// https://github.com/mfichman/http

#pragma once

#include "Response.hpp"
#include "Request.hpp"

namespace inl::net::http
{
	class Http
	{
	public:
		static Response Get(std::string const& path, std::string const& data = "");
		static Response Post(std::string const& path, std::string const& data = "");

	private:
		static Response Send(Request const& request);
		static std::string Str(Request const& request);
	};
}