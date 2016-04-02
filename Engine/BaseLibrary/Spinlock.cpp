#include "Spinlock.hpp"

#include <cassert>
#include <stdexcept>
#include <iostream>


namespace exc {


spin_mutex::spin_mutex() {
	flag = false;
}

void spin_mutex::lock() {
	bool expected;
	do {
		while (flag != false)
			;
		expected = false;
		flag.compare_exchange_strong(expected, true);
	} while (expected);
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
		throw std::logic_error("Unlock must be called from the locking thread.");
	}
}


} // namespace exc