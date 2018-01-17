// https://github.com/mfichman/http

#pragma once

#include "Uri.hpp"
#include "Headers.hpp"
#include "Enums.hpp"

namespace inl::net::http
{
	using namespace enums;

	class Request 
	{
	public:
		Method GetMethod() const 
		{ 
			return m_method;
		}

		const Uri& GetUri() const 
		{ 
			return m_uri;
		}

		const std::string& GetPath() const 
		{ 
			return m_uri.GetPath();
		}

		const std::string& GetData() const 
		{ 
			return m_data;
		}

		const std::string GetHeaderElement(const std::string& name) const;

		const Headers& GetHeaders() const 
		{ 
			return m_headers; 
		}

		void SetMethod(Method method);
		void SetUri(const Uri& path);
		void SetData(const std::string& data);
		void AddHeader(const std::string& name, const std::string& value);

	private:
		Method m_method = Method::GET;
		Uri m_uri;
		std::string m_data;
		Headers m_headers;
	};
}