#pragma once

#include <queue>
#include <mutex>

#include "NetworkMessage.hpp"

#include "NewConnectionEvent.hpp"
#include "DisconnectedEvent.hpp"
#include "DataReceivedEvent.hpp"

namespace inl::net
{
	using namespace events;

	class MessageQueue
	{
	public:
		MessageQueue()
		{
		}

		void EnqueueMessageToSend(const NetworkMessage &msg);

		void EnqueueMessageReceived(const NetworkMessage &msg);
		void EnqueueDisconnection(const NetworkMessage &msg);
		void EnqueueConnection(const NetworkMessage &msg);

		NetworkMessage DequeueMessageToSend();

		uint32_t SendSize();

	private:
		std::deque<NetworkMessage> m_messagesToSend;

		std::deque<NewConnectionEvent> m_connectionEvents;
		std::deque<DisconnectedEvent> m_disconnectedEvents;
		std::deque<DataReceivedEvent> m_dataReceivedEvents;

		std::mutex m_sendMutex;
		std::mutex m_receivedMutex;
		std::mutex m_disconnectMutex;
		std::mutex m_connectionMutex;
	};
}