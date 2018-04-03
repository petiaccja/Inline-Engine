#include "SpinMutex.hpp"
#include "Exception/Exception.hpp"

#include <cassert>
#include <stdexcept>
#include <iostream>


namespace inl {


spin_mutex::spin_mutex() {
	flag = false;
}

void spin_mutex::lock() {
	bool expected;
	bool success;
	do {
		expected = false;
		success = flag.compare_exchange_weak(expected, true);
	} while (!success);
	ownerId = std::this_thread::get_id();
}

bool spin_mutex::try_lock() {
	bool expected = false;
	if (flag == false && flag.compare_exchange_strong(expected, true)) {
		ownerId = std::this_thread::get_id();
		return true;
	}
	else {
		return false;
	}
}

void spin_mutex::unlock() {
	if (ownerId == std::this_thread::get_id()) {
		bool expected = true;
		flag.compare_exchange_strong(expected, false);
	}
	else {
		throw InvalidCallException("Unlock must be called from the locking thread.");
	}
}


} // namespace inl