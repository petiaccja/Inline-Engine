#pragma once

#include <vector>
#include <sstream>

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
}