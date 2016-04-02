#pragma once

#include <cstdint>
#include <string>

namespace exc {
	

class InputStream {
public:
	virtual bool CanSeek() = 0;
	virtual void SeekRead(size_t position) = 0;
	virtual size_t TellRead() = 0;
	virtual size_t Length() = 0;

	virtual size_t Read(void* buffer, size_t bytesToRead) = 0;
	virtual char ReadChar() = 0;
	virtual std::string ReadChars() = 0;


	virtual InputStream& operator>>(int);





};


class OutputStream {
public:



};



} // namespace exc