#include "SpinMutex.hpp"
#include "Exception/Exception.hpp"

#include <cassert>
#include <stdexcept>
#include <iostream>


namespace inl {


SpinMutex::SpinMutex() {
	flag = false;
}

SpinMutex::SpinMutex(SpinMutex&& rhs) noexcept 
	: ownerId(rhs.ownerId.load()), flag(rhs.flag.load())
{
	rhs.flag.store(false);
}

SpinMutex& SpinMutex::operator=(SpinMutex&& rhs) noexcept {
	ownerId = rhs.ownerId.load();
	flag = rhs.flag.load();
	rhs.flag.store(false);
	return *this;
}

void SpinMutex::lock() {
	bool expected;
	bool success;
	do {
		expected = false;
		success = flag.compare_exchange_weak(expected, true);
	} while (!success);
	ownerId = std::this_thread::get_id();
}

bool SpinMutex::try_lock() {
	bool expected = false;
	if (flag == false && flag.compare_exchange_strong(expected, true)) {
		ownerId = std::this_thread::get_id();
		return true;
	}
	else {
		return false;
	}
}

void SpinMutex::unlock() {
	if (ownerId == std::this_thread::get_id()) {
		bool expected = true;
		flag.compare_exchange_strong(expected, false);
	}
	else {
		throw InvalidCallException("Unlock must be called from the locking thread.");
	}
}


} // namespace inl