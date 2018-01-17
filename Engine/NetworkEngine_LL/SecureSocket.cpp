#include "SecureSocket.hpp"

#include <BaseLibrary/Exception/Exception.hpp>

#include <cassert>

namespace inl::net::sockets
{
	SecureSocket::SecureSocket()
		: m_context(0), m_conn(0), m_eof(false) 
	{
		m_socket = std::make_unique<Socket>(SocketType::Streaming);

		// Intitialize the SSL client-side socket
		SSL_library_init();
		SSL_load_error_strings();
		ERR_load_BIO_strings();
	}

	bool SecureSocket::Connect(const IPAddress & addrStr)
	{
		if (!m_context) 
		{
			m_context = SSL_CTX_new(SSLv23_client_method());
			assert(m_context);
		}
		if (!m_conn) 
		{
			m_conn = SSL_new(m_context);
			assert(m_conn);

			m_in = BIO_new(BIO_s_mem());
			m_out = BIO_new(BIO_s_mem());
			SSL_set_bio(m_conn, m_in, m_out);
			SSL_set_connect_state(m_conn);
		}

		return m_socket->Connect(addrStr);
	}

	bool SecureSocket::Close() const
	{
		return m_socket->Close();
	}

	bool SecureSocket::HasPendingData(uint32_t & pendingDataSize) const
	{
		return m_socket->HasPendingData(pendingDataSize);
	}

	bool SecureSocket::Send(uint8_t* data, int32_t count, int32_t &sent, int flags)
	{
		sent = SSL_write(m_conn, data, count);
		SendFromBio(); // Write data if available
		if (sent < 0) 
		{
			HandleReturn(sent);
			return true;
		}
		return sent > 0;
	}

	bool SecureSocket::SendRaw(uint8_t * buf, size_t len, int flags)
	{
		int32_t sent;
		return m_socket->Send(buf, len, sent) && sent == len;
		
	}

	bool SecureSocket::SendFromBio(int flags)
	{
		uint8_t buf[4096];
		size_t pending = BIO_ctrl_pending(m_out);
		if (!pending) 
			return true;
		size_t bytes = BIO_read(m_out, buf, sizeof(buf));
		if (bytes > 0) 
			return SendRaw(buf, bytes, flags);
		else if (bytes == -1 || bytes == 0)
			return true;
		return false;
	}

	bool SecureSocket::RecvToBio(int flags)
	{
		uint8_t buf[4096];
		size_t bytes = m_socket->Recv(buf, sizeof(buf), flags);
		if (bytes > 0) 
		{
			size_t written = BIO_write(m_in, buf, int(bytes));
			assert(bytes == written);
			return true;
		}
		else if (bytes == 0) 
		{
			// No data
			m_eof = true;
			return true;
		}
		return false;
	}

	void SecureSocket::HandleReturn(size_t ret)
	{
		int32_t err = SSL_get_error(m_conn, ret);
		if (SSL_ERROR_WANT_WRITE == err)
			SendFromBio();
		else if (SSL_ERROR_WANT_READ == err)
			RecvToBio();
		else if (SSL_ERROR_SSL == err)
			throw inl::RuntimeException();
		else
			assert(!"unexpected error");
	}

	bool SecureSocket::Recv(uint8_t* data, int32_t count, int32_t &read, int flags)
	{
		read = SSL_read(m_conn, data, count);
		if (read < 0) 
		{
			HandleReturn(read);
			if (m_eof) 
				return false;
		}
		return read > 0;
	}

	bool SecureSocket::Wait(SocketWaitConditions cond, std::chrono::milliseconds t) const
	{
		return m_socket->Wait(cond, t);
	}

	SocketConnectionState SecureSocket::GetConnectionState() const
	{
		return m_socket->GetConnectionState();
	}

	void SecureSocket::GetAddress(IPAddress & outAddr) const
	{
		m_socket->GetAddress(outAddr);
	}

	int32_t SecureSocket::GetPort() const
	{
		return m_socket->GetPort();
	}

	void SecureSocket::UseCertificateFile(std::string const & path)
	{
		if (!m_context) 
			assert(!"not initialized yet");
		if (SSL_CTX_use_certificate_file(m_context, path.c_str(), SSL_FILETYPE_PEM) <= 0) 
			throw inl::RuntimeException();
	}

	void SecureSocket::UsePrivateKeyFile(std::string const & path)
	{
		if (!m_context)
			assert(!"not initialized yet");
		if (SSL_CTX_use_PrivateKey_file(m_context, path.c_str(), SSL_FILETYPE_PEM) <= 0)
			throw inl::RuntimeException();
		if (!SSL_CTX_check_private_key(m_context))
			throw inl::RuntimeException();
	}
}