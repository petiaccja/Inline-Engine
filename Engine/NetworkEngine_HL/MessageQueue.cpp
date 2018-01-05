#include "MessageQueue.hpp"
namespace inl::net
{
	void MessageQueue::EnqueueMessageToSend(const NetworkMessage & msg)
	{
		m_sendMutex.lock();
		m_messagesToSend.push(msg);
		m_sendMutex.unlock();
	}

	void MessageQueue::EnqueueMessageReceived(const NetworkMessage & msg)
	{
		m_receivedMutex.lock();
		m_messagesReceived.push(msg);
		m_receivedMutex.unlock();
	}

	NetworkMessage MessageQueue::DequeueMessageToSend()
	{
		m_sendMutex.lock();
		NetworkMessage msg = m_messagesToSend.front();
		m_messagesToSend.pop();
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