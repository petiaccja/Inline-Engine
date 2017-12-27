#pragma once

#include <vector>
#include <sstream>

#include "Net.hpp"

namespace inl::net::util
{
	inline static std::vector<std::string> Split(const std::string &str, const std::string &delimiter)
	{
		std::vector<std::string> splited;
		if (str.empty() && delimiter.empty())
			return std::vector<std::string>();
		std::string::size_type lastPos = str.find_first_not_of(delimiter, 0);
		std::string::size_type pos = str.find_first_of(delimiter, lastPos);

		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			splited.push_back(str.substr(lastPos, pos - lastPos));
			lastPos = str.find_first_not_of(delimiter, pos);
			pos = str.find_first_of(delimiter, lastPos);
		}
		return splited;
	}

	sockaddr_in CreateAddress(uint32_t address, uint16_t port)
	{
		sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_addr.s_addr = htonl(address);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);

		return addr;
	}
}