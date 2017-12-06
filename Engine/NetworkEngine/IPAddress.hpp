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

    IPAddress();
    IPAddress(const std::string& address, uint16_t port = DEFAULT_SERVER_PORT);
    IPAddress(const char* address, uint16_t port = DEFAULT_SERVER_PORT);
    IPAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint16_t port = DEFAULT_SERVER_PORT);
    explicit IPAddress(uint32_t address, uint16_t port = DEFAULT_SERVER_PORT);

    std::string ToString() const;
    uint32_t ToInteger() const;
	uint16_t GetPort() const;

    static IPAddress GetLocalAddress();
    static IPAddress GetPublicAddress(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    static const IPAddress None;
    static const IPAddress Any;
    static const IPAddress LocalHost;
    static const IPAddress Broadcast;

private:

    friend bool operator <(const IPAddress& left, const IPAddress& right);

    void Resolve(const std::string& address);

    uint32_t m_address;
    bool m_valid;
	uint16_t m_port;
};

bool operator ==(const IPAddress& left, const IPAddress& right);
bool operator !=(const IPAddress& left, const IPAddress& right);
bool operator <(const IPAddress& left, const IPAddress& right);
bool operator >(const IPAddress& left, const IPAddress& right);
bool operator <=(const IPAddress& left, const IPAddress& right);
bool operator >=(const IPAddress& left, const IPAddress& right);
std::istream& operator >>(std::istream& stream, IPAddress& address);
std::ostream& operator <<(std::ostream& stream, const IPAddress& address);