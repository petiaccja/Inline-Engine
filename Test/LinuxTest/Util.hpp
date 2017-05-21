#pragma once

#include <vector>
#include <sstream>

namespace inl::net::util
{
	template<class T>
	inline static void Delete(T *ptr)
	{
		delete ptr;
		ptr = nullptr;
	}

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

	inline static bool ValidIPv4Addr(const std::string &addr)
	{
		std::vector<std::string> splitted_address = Split(addr, ".");
		if (splitted_address.size() != 4)
			return false;
		try
		{
			int a1 = std::stoi(splitted_address[0].c_str());
			int a2 = std::stoi(splitted_address[1].c_str());
			int a3 = std::stoi(splitted_address[2].c_str());
			int a4 = std::stoi(splitted_address[3].c_str());

			return a1 >= 0 && a2 >= 0 && a3 >= 0 && a4 >= 0 &&
				a1 < 255 && a2 < 255 && a3 < 255 && a4 < 255;
		}
		catch (std::exception)
		{
			return false;
		}
	}

	template<class To, class From>
	To CastBytes(From v)
	{
		return static_cast<To>(static_cast<void*>(v));
	}

	inline char *IntToStr(int n)
	{
		char *str = new char[sizeof(int)]();
		snprintf(str, sizeof(int), "%d", n);
		return str;
	}
}