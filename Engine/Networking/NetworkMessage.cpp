#include "NetworkMessage.hpp"

namespace inl::net
{
	NetworkBuffer inl::net::NetworkMessage::EncodeMessage(const NetworkMessage & message)
	{
		NetworkBuffer net_buffer;
		try
		{
			char *sender_id = itoa(message.SenderID, new char[0](), 10);
			char *dist_mode = itoa(message.DistributionMode, new char[0](), 10);
			char *dest_id = itoa(message.DestinationID, new char[0](), 10);
			char *tag = itoa(message.Tag, new char[0](), 10);
			char *subject = itoa(message.Subject, new char[0](), 10);

			net_buffer.BodySize = 20;
			strcat(net_buffer.Body, sender_id);
			strcat(net_buffer.Body, dist_mode);
			strcat(net_buffer.Body, dest_id);
			strcat(net_buffer.Body, tag);
			strcat(net_buffer.Body, subject);
			strcat(net_buffer.Body, static_cast<char*>(message.Data));
			net_buffer.Valid = true;
		}
		catch (std::exception)
		{
			net_buffer.Valid = false;
		}

		return net_buffer;
	}

	NetworkMessage NetworkMessage::DecodeMessage(const NetworkBuffer & buffer)
	{
		NetworkMessage message;
		try
		{
			message.SenderID = atoi(strncpy(new char[0](), buffer.Body, sizeof(int)));
			message.DistributionMode = atoi(strncpy(new char[0](), buffer.Body + sizeof(int), sizeof(int)));
			message.DestinationID = atoi(strncpy(new char[0](), buffer.Body + sizeof(int) * 2, sizeof(int)));
			message.Tag = atoi(strncpy(new char[0](), buffer.Body + sizeof(int) * 3, sizeof(int)));
			message.Subject = atoi(strncpy(new char[0](), buffer.Body + sizeof(int) * 4, sizeof(int)));
			message.Data = static_cast<void*>(strncpy(new char[0](), buffer.Body + sizeof(int) * 5, buffer.BodySize - 20));
			message.Valid = true;
		}
		catch (std::exception)
		{
			message.Valid = false;
		}
		return message;
	}
}

// you have to convert all of the member variables of the variable message of type NetworkMessage into character arrays (char *) and 
// then you concat them probably using strcat we'll see later xD