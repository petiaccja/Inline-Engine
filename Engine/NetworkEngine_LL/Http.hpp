// https://github.com/mfichman/http

#pragma once

#include "Response.hpp"
#include "Request.hpp"

namespace inl::net::http
{
	class Http
	{
	public:
		static Response get(std::string const& path, std::string const& data = "");
		static Response post(std::string const& path, std::string const& data = "");

	private:
		static Response send(Request const& request);
		static std::string str(Request const& request);
	};
}