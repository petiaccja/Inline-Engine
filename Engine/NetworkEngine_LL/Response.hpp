// https://github.com/mfichman/http

#pragma once

#include "Headers.hpp"
#include "Cookies.hpp"

#include <string>

namespace inl::net::http
{
	class Response 
	{
	public:
		enum Status 
		{
			INVALID_CODE = 0,
			CONTINUE = 100,
			SWITCHING_PROTOCOLS = 101,
			OK = 200,
			CREATED = 201,
			ACCEPTED = 202,
			NON_AUTHORITATIVE_INFO = 203,
			NO_CONTENT = 204,
			RESET_CONTENT = 205,
			PARTIAL_CONTENT = 206,
			MULTIPLE_CHOICES = 300,
			MOVED_PERMANENTLY = 301,
			FOUND = 302,
			SEE_OTHER = 303,
			NOT_MODIFIED = 304,
			USE_PROXY = 305,
			TEMPORARY_REDIRECT = 307,
			BAD_REQUEST = 400,
			UNAUTHORIZED = 401,
			PAYMENT_REQUIRED = 402,
			FORBIDDEN = 403,
			NOT_FOUND = 404,
			METHOD_NOT_ALLOWED = 405,
			NOT_ACCEPTABLE = 406,
			PROXY_AUTHENTICATION_REQUIRED = 407,
			REQUEST_TIMEOUT = 408,
			CONFLICT = 409,
			GONE = 410,
			LENGTH_REQUIRED = 411,
			PRECONDITION_FAILED = 412,
			REQUEST_ENTITY_TOO_LARGE = 413,
			UNSUPPORTED_MEDIA_TYPE = 415,
			REQUESTED_RANGE_NOT_SATISFIABLE = 416,
			EXPECTATION_FAILED = 417,
			INTERNAL_SERVER_ERROR = 500,
			NOT_IMPLEMENTED = 501,
			BAD_GATEWAY = 502,
			SERVICE_UNAVAILABLE = 503,
			GATEWAY_TIMEOUT = 504,
			VERSION_NOT_SUPPORTED = 505,
		};

		Response(std::string const& text);
		Response() {};

		Status status() const 
		{ 
			return status_; 
		}

		std::string const& data() const 
		{ 
			return data_; 
		}

		std::string const header(std::string const& name) const;
		Cookie const cookie(std::string const& name) const;

		void statusIs(Status status);
		void versionIs(std::string const& version);
		void dataIs(std::string const& data);
		void headerIs(std::string const& name, std::string const& value);
		void cookieIs(Cookie const& cookie);

	private:
		Status status_ = INVALID_CODE;
		std::string data_;
		Headers headers_;
		Cookies cookies_;
	};
}