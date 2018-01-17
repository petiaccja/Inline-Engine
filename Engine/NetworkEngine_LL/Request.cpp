// https://github.com/mfichman/http

#include "Request.hpp"

namespace inl::net::http
{
	const std::string Request::GetHeaderElement(const std::string& name) const 
	{
		return m_headers[name];
	}

	void Request::SetMethod(Method method) 
	{
		m_method = method;
	}

	void Request::SetUri(const Uri& uri) 
	{
		m_uri = uri;
	}

	void Request::SetData(const std::string& data) 
	{
		m_data = data;
	}

	void Request::AddHeader(const std::string& name, const std::string& value) 
	{
		m_headers.AddHeader(name, value);
	}
}