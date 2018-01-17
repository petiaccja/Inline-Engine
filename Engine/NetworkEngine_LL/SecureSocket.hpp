#pragma once

#include "Socket.hpp"

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace inl::net
{
	using namespace enums;
	using namespace sockets;

	class SecureSocket
	{
	public:
		SecureSocket(Socket *soc);

		bool Connect(const IPAddress& addr);
		bool Close() const;
		bool HasPendingData(uint32_t& pendingDataSize) const;
		bool Send(uint8_t* data, int32_t count, int32_t &sent, int flags = 0); // Execute 1 write() syscall
		bool Recv(uint8_t* data, int32_t count, int32_t &read, int flags = 0); // Execte 1 read() syscall
		bool Wait(SocketWaitConditions cond, std::chrono::milliseconds t) const;
		SocketConnectionState GetConnectionState() const;
		void GetAddress(IPAddress& outAddr) const;
		int32_t GetPort() const;

		void UseCertificateFile(std::string const& path);
		void UsePrivateKeyFile(std::string const& path);

	private:
		bool SendRaw(uint8_t* buf, size_t len, int flags = 0);
		bool SendFromBio(int flags = 0);
		bool RecvToBio(int flags = 0);
		void HandleReturn(size_t ret);

		std::unique_ptr<Socket> m_socket;

		SSL_CTX* m_context;
		SSL* m_conn;
		BIO* m_in;
		BIO* m_out;
		bool m_eof;
	};
}