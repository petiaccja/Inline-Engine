#include "MessageQueue.hpp"

namespace inl::net
{
	void MessageQueue::EnqueueMessageToSend(const NetworkMessage & msg)
	{
		m_sendMutex.lock();
		m_messagesToSend.emplace_back(msg);
		m_sendMutex.unlock();
	}

	void MessageQueue::EnqueueMessageReceived(const NetworkMessage & msg)
	{
		m_receivedMutex.lock();
		DataReceivedEvent ev(msg);
		m_dataReceivedEvents.push_back(ev);
		m_receivedMutex.unlock();
	}

	void MessageQueue::EnqueueDisconnection(const NetworkMessage & msg)
	{
		m_disconnectMutex.lock();
		std::unique_ptr<DisconnectedEvent> ev = std::make_unique<DisconnectedEvent>(((NetworkMessage)msg).GetData<DisconnectedEvent>());
		m_disconnectedEvents.push_back(*(ev.get()));
		m_disconnectMutex.unlock();
	}

	void MessageQueue::EnqueueConnection(const NetworkMessage & msg)
	{
		m_connectionMutex.lock();
		NewConnectionEvent ev(msg.m_senderID, (uint8_t*)msg.m_data);
		m_connectionEvents.push_back(ev);
		m_connectionMutex.unlock();
	}

	NetworkMessage MessageQueue::DequeueMessageToSend()
	{
		m_sendMutex.lock();
		NetworkMessage msg = m_messagesToSend.front();
		m_messagesToSend.erase(m_messagesToSend.begin() + 1);
		m_sendMutex.unlock();
		return msg;
	}

	uint32_t MessageQueue::SendSize()
	{
		m_sendMutex.lock();
		uint32_t size = m_messagesToSend.size();
		m_sendMutex.unlock();
		return size;
	}
}