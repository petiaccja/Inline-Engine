#include "IPAddress.hpp"
#include "Util.hpp"

const IPAddress IPAddress::None;
const IPAddress IPAddress::Any(0, 0, 0, 0);
const IPAddress IPAddress::LocalHost(127, 0, 0, 1);
const IPAddress IPAddress::Broadcast(255, 255, 255, 255);

IPAddress::IPAddress() 
	: m_address(0), m_valid(false), m_port (DEFAULT_SERVER_PORT)
{
}

IPAddress::IPAddress(const std::string& address, uint16_t port) 
	: m_address(0), m_valid(false), m_port(port)
{
    Resolve(address);
}

IPAddress::IPAddress(const char* address, uint16_t port) 
	: m_address(0), m_valid(false), m_port(port)
{
    Resolve(address);
}

IPAddress::IPAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint16_t port) 
	: m_address(htonl((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3)), m_valid(true), m_port(port)
{
}

IPAddress::IPAddress(uint32_t address, uint16_t port) 
	: m_address(htonl(address)), m_valid(true), m_port(port)
{
}

std::string IPAddress::ToString() const
{
    in_addr address;
    address.s_addr = m_address;

    return inet_ntoa(address);
}

uint32_t IPAddress::ToInteger() const
{
    return ntohl(m_address);
}

uint16_t IPAddress::GetPort() const
{
	return m_port;
}

IPAddress IPAddress::GetPublicAddress(std::chrono::milliseconds timeout)
{
    return IPAddress();
}

void IPAddress::Resolve(const std::string& address)
{
    m_address = 0;
    m_valid = false;

    if (address == "255.255.255.255")
    {
        // The broadcast address needs to be handled explicitly,
        // because it is also the value returned by inet_addr on error
        m_address = INADDR_BROADCAST;
        m_valid = true;
    }
    else if (address == "0.0.0.0")
    {
        m_address = INADDR_ANY;
        m_valid = true;
    }
    else
    {
        uint32_t ip = inet_addr(address.c_str());
        if (ip != INADDR_NONE)
        {
            m_address = ip;
            m_valid = true;
        }
        else
        {
            addrinfo hints;
            std::memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            addrinfo* result = NULL;
            if (getaddrinfo(address.c_str(), NULL, &hints, &result) == 0)
            {
                if (result)
                {
                    ip = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
                    freeaddrinfo(result);
                    m_address = ip;
                    m_valid = true;
                }
            }
        }
    }
}

bool operator ==(const IPAddress& left, const IPAddress& right)
{
    return !(left < right) && !(right < left);
}

bool operator !=(const IPAddress& left, const IPAddress& right)
{
    return !(left == right);
}

bool operator <(const IPAddress& left, const IPAddress& right)
{
    return std::make_pair(left.m_valid, left.m_address) < std::make_pair(right.m_valid, right.m_address);
}

bool operator >(const IPAddress& left, const IPAddress& right)
{
    return right < left;
}

bool operator <=(const IPAddress& left, const IPAddress& right)
{
    return !(right < left);
}

bool operator >=(const IPAddress& left, const IPAddress& right)
{
    return !(left < right);
}

std::istream& operator >>(std::istream& stream, IPAddress& address)
{
    std::string str;
    stream >> str;
    address = IPAddress(str);

    return stream;
}

std::ostream& operator <<(std::ostream& stream, const IPAddress& address)
{
    return stream << address.ToString();
}