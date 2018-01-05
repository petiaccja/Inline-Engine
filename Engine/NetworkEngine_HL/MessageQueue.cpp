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
		m_messagesReceived.emplace_back(msg);
		m_receivedMutex.unlock();
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