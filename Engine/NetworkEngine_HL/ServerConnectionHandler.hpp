#pragma once

#include <queue>

#include <SpinMutex.hpp>
#include <Exception/Exception.hpp>
#include <Serialization/BinarySerializer.hpp>

#include <TcpClient.hpp>

#include "NewConnectionEvent.hpp"
#include "DisconnectedEvent.hpp"
#include "ServerConnection.hpp"

namespace inl::net::servers
{
	using namespace sockets;
	using namespace events;

	class ServerConnectionHandler
	{
		friend class ServerConnection;

	public:
		ServerConnectionHandler()
		{
		}

		void Add(ServerConnection *c)
		{
			uint32_t id = GetAvailableID();
			if (id == -1)
			{
				// this can be handled just by the server
				// what if the server owner wants to know if a user wanted to join but couldnt
				DisconnectedEvent disconnected_event(id, "Server Full", -1);
				TcpClient *client = c->GetClient();
				int32_t size = 0;
				uint8_t *buffer = disconnected_event.Serialize(size);
				int32_t sent = 0;
				client->Send(buffer, size, sent);
				client->Close();
			}

			c->SetID(id);

			m_list.push_back(c);
			NewConnectionEvent *new_conn_event = new NewConnectionEvent(c->GetClient(), id);
			// send new connection event to main thread - but how
		}

		uint32_t GetAvailableID()
		{
			for (int i = 1; i <= m_maxConnections; i++)
			{
				bool flag = true;
				for (int k = 0; k < m_list.size(); k++)
				{
					if (m_list.at(k)->GetID() == i)
						flag = false;
				}

				if (flag)
					return i;
			}

			//throw OutOfRangeException("Out of IDs to allocate - clients = max connections", "NewConnectionEventPool");
			return -1;
		}

		void SetMaxConnections(uint32_t max_connections)
		{
			m_maxConnections = max_connections;
		}

	private:
		std::vector<ServerConnection*> m_list;
		uint32_t m_maxConnections;
	};
}