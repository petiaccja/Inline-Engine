#pragma once

#include "UdpSocket.hpp"
#include "IPAddress.hpp"

#include "BaseLibrary/Exception/Exception.hpp"

#include <vector>

namespace inl::net::sockets
{
	class UdpSocketBuilder
	{
	public:
		UdpSocketBuilder()
			: m_blocking(false)
			, m_bound(false)
			, m_boundEndpoint(IPAddress::Any, 0)
			, m_multicastLoopback(false)
			, m_multicastTtl(1)
			, m_receiveBufferSize(0)
			, m_reusable(false)
			, m_sendBufferSize(0)
		{ }

	public:
		UdpSocketBuilder AsBlocking()
		{
			m_blocking = true;

			return *this;
		}

		UdpSocketBuilder AsNonBlocking()
		{
			m_blocking = false;

			return *this;
		}

		UdpSocketBuilder AsReusable()
		{
			m_reusable = true;

			return *this;
		}

		UdpSocketBuilder BoundToAddress(const IPAddress& addr)
		{
			m_boundEndpoint = IPAddress(addr);
			m_bound = true;

			return *this;
		}

		UdpSocketBuilder BoundToPort(uint16_t port)
		{
			m_boundEndpoint = IPAddress(m_boundEndpoint.ToInteger(), port);
			m_bound = true;

			return *this;
		}

		UdpSocketBuilder JoinedToGroup(const IPAddress& group_addr)
		{
			m_joinedGroups.emplace_back(group_addr);

			return *this;
		}

		UdpSocketBuilder WithMulticastLoopback()
		{
			m_multicastLoopback = true;

			return *this;
		}

		UdpSocketBuilder WithMulticastTtl(uint8_t time_to_live)
		{
			m_multicastTtl = time_to_live;

			return *this;
		}

		UdpSocketBuilder WithReceiveBufferSize(uint32_t size)
		{
			m_receiveBufferSize = size;

			return *this;
		}

		UdpSocketBuilder WithSendBufferSize(uint32_t size)
		{
			m_sendBufferSize = size;

			return *this;
		}

	public:
		std::unique_ptr<UdpSocket> Build() const
		{
			std::unique_ptr<Socket> soc = std::make_unique<Socket>(SocketType::Datagram);
			
			if (soc)
			{
				bool Error =
					!soc->SetNonBlocking(!m_blocking) ||
					!soc->SetReuseAddr(m_reusable);

				if (!Error)
					Error = m_bound && !soc->Bind(m_boundEndpoint);
				if (!Error)
					Error = !soc->SetMulticastLoopback(m_multicastLoopback) || !soc->SetMulticastTtl(m_multicastTtl);

				if (!Error)
				{
					for (const auto& Group : m_joinedGroups)
					{
						if (!soc->JoinMulticastGroup(IPAddress(Group, 0)))
						{
							Error = true;
							break;
						}
					}
				}

				if (!Error)
				{
					int32_t out_new_size;
					if (m_receiveBufferSize > 0)
						soc->SetReceiveBufferSize(m_receiveBufferSize, out_new_size);
					if (m_sendBufferSize > 0)
						soc->SetSendBufferSize(m_sendBufferSize, out_new_size);
				}

				if (Error)
					throw inl::RuntimeException("Couldnt create socket"); // make parameter a string depending on the error
				return std::make_unique<UdpSocket>();
			}
			return std::unique_ptr<UdpSocket>(nullptr);
		}

	private:
		bool m_blocking;
		bool m_bound;
		IPAddress m_boundEndpoint;
		std::vector<IPAddress> m_joinedGroups;
		bool m_multicastLoopback;
		uint8_t m_multicastTtl;
		uint32_t m_receiveBufferSize;
		bool m_reusable;
		uint32_t m_sendBufferSize;
	};
}