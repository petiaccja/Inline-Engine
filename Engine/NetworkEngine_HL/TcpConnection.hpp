#pragma once

#include "TcpClient.hpp"

#include "NetworkMessage.hpp"

#include "BaseLibrary/Event.hpp"

namespace inl::net
{
	using namespace sockets;

	class TcpConnection
	{
	public:
		TcpConnection(TcpClient *client);

		std::shared_ptr<TcpClient> GetClient();
		uint32_t GetID();
		void SetID(uint32_t id);

		template<typename T>
		void SendMessage(DistributionMode mode, uint32_t destinationId, uint32_t tag, void *data)
		{
			NetworkMessage msg(m_id, mode, destinationId, tag, data, sizeof(T));
		}

		void ReceiveData();

		inl::Event<uint32_t, DistributionMode, uint32_t, uint32_t, void*> DataReceivedEvent;
		inl::Event<uint32_t, std::string, int32_t> DisconnectedEvent;
		inl::Event<uint32_t, void*> NewConnectionEvent;

	private:
		bool sendMessage(NetworkMessage &msg);

		std::shared_ptr<TcpClient> m_client;
		uint32_t m_id;
	};
}