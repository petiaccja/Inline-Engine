// https://github.com/mfichman/http

#pragma once

#include "Headers.hpp"
#include "Cookies.hpp"
#include "Enums.hpp"

#include <string>

namespace inl::net::http
{
	using namespace enums;

	class Response 
	{
	public:
		Response(const std::string& text);
		Response() {};

		HttpStatus GetStatus() const 
		{ 
			return m_status; 
		}

		const std::string& daGetDatata() const 
		{ 
			return m_data; 
		}

		const std::string GetHeader(const std::string& name) const;
		const Cookie GetCookie(const std::string& name) const;

		void SetStatus(HttpStatus status);
		void SetData(const std::string& data);
		void SetHeader(const std::string& name, const std::string& value);
		void SetCookie(const Cookie& cookie);

	private:
		HttpStatus m_status = HttpStatus::INVALID_CODE;
		std::string m_data;
		Headers m_headers;
		Cookies m_cookies;
	};
}