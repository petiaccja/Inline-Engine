#pragma once 

//#include <SFML/Network/Export.hpp>
//#include <SFML/System/Time.hpp>
#include "Net.hpp"
#include <chrono>
#include <istream>
#include <ostream>
#include <string>

class IPAddress
{
public:
	inline IPAddress()
		: m_address(0)
		, m_valid(false)
		, m_port(DEFAULT_SERVER_PORT)
	{
	}

	inline IPAddress(const std::string& address, uint16_t port = DEFAULT_SERVER_PORT)
		: m_address(0)
		, m_valid(false)
		, m_port(port)
	{
		Resolve(address);
	}

	inline IPAddress(const char* address, uint16_t port = DEFAULT_SERVER_PORT)
		: m_address(0)
		, m_valid(false)
		, m_port(port)
	{
		Resolve(address);
	}

	inline IPAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint16_t port = DEFAULT_SERVER_PORT)
		: m_address(htonl((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3))
		, m_valid(true)
		, m_port(port)
	{
	}

	inline explicit IPAddress(uint32_t address, uint16_t port = DEFAULT_SERVER_PORT)
		: m_address(htonl(address))
		, m_valid(true)
		, m_port(port)
	{
	}

	std::string ToString() const;
	inline uint32_t ToInteger() const { return ntohl(m_address); }
	inline uint16_t GetPort() const { return m_port; }

    static const IPAddress None;
    static const IPAddress Any;
    static const IPAddress LocalHost;
    static const IPAddress Broadcast;

private:

    friend bool operator <(const IPAddress& left, const IPAddress& right);

    void Resolve(const std::string& address);

private:
    uint32_t m_address;
    bool m_valid;
	uint16_t m_port;
};

inline bool operator ==(const IPAddress& left, const IPAddress& right) { return !(left < right) && !(right < left); }
inline bool operator !=(const IPAddress& left, const IPAddress& right) { return !(left == right); }
inline bool operator <(const IPAddress& left, const IPAddress& right) { return std::make_pair(left.m_valid, left.m_address) < std::make_pair(right.m_valid, right.m_address); }
inline bool operator >(const IPAddress& left, const IPAddress& right) { return right < left; }
inline bool operator <=(const IPAddress& left, const IPAddress& right) { return !(right < left); }
inline bool operator >=(const IPAddress& left, const IPAddress& right) { return !(left < right); }

inline std::istream& operator >>(std::istream& stream, IPAddress& address)
{
	std::string str;
	stream >> str;
	address = IPAddress(str);

	return stream;
}

inline std::ostream& operator <<(std::ostream& stream, const IPAddress& address) { return stream << address.ToString(); }