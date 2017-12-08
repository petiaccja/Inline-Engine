#include "IPAddress.hpp"
#include "Util.hpp"

const IPAddress IPAddress::None;
const IPAddress IPAddress::Any(0, 0, 0, 0);
const IPAddress IPAddress::LocalHost(127, 0, 0, 1);
const IPAddress IPAddress::Broadcast(255, 255, 255, 255);

std::string IPAddress::ToString() const
{
    in_addr address;
    address.s_addr = m_address;

    return inet_ntoa(address);
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