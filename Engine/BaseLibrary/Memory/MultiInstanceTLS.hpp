#pragma once

#include "SlabAllocatorEngine.hpp"

#include <map>
#include <algorithm>
#include <mutex>


// optional is part of C++17, but not included in VS yet
// this is a reference implementation by the authors of the proposal
// uses boost license
// replace when official is available
// see https://github.com/akrzemi1/Optional
#define TR2_OPTIONAL_DISABLE_EMULATION_OF_TYPE_TRAITS
#include "../optional.hpp"
namespace std {
	template <class T>
	using optional = experimental::optional<T>;
}

namespace exc {


template <class T>
class mi_tls {
public:
	mi_tls() {
		AllocateSlot();
	}

	~mi_tls() {
		// deallocate our slot
		std::lock_guard<std::mutex> lkg(allocatorLock);

		allocator.Deallocate(myIndex);
	}

	// Construct from in-place arguments
	template <class... Args>
	mi_tls(Args&&... args) : defaultRecord(std::forward<Args>(args)...) {
		AllocateSlot();
	}

	// Construct from data
	mi_tls(const T& value) : defaultRecord(value) {
		AllocateSlot();
	}

	mi_tls(T&& value) : defaultRecord(std::move(value)) {
		AllocateSlot();
	}

	// Copy ctors
	mi_tls(const mi_tls& rhs) : mi_tls(rhs.GetRef()) {}
	mi_tls(mi_tls&& rhs) : mi_tls(std::move(rhs.GetRef())) {}


	// Assign operators
	mi_tls& operator=(const T& value) {
		GetRef() = value;
		return *this;
	}

	mi_tls& operator=(T&& value) {
		GetRef() = std::move(value);
		return *this;
	}

	mi_tls& operator=(const mi_tls& rhs) {
		GetRef() = rhs.GetRef();
		return *this;
	}

	mi_tls& operator=(mi_tls&& rhs) {
		GetRef() = std::move(rhs.GetRef());
		return *this;
	}
	

	// Convert to underlying type
	operator T&() & {
		return GetRef();
	}
	operator const T&() const& {
		return GetRef();
	}
	operator T && () && {
		return std::move(GetRef());
	}
private:
	void AllocateSlot() {
		std::lock_guard<std::mutex> lkg(allocatorLock);

		try {
			myIndex = allocator.Allocate();
		}
		catch (std::bad_alloc&) {
			size_t currentSize = allocator.Size();
			allocator.Resize(currentSize * 1.2);
			myIndex = allocator.Allocate(); // supposed to have enough space now
		}
	}

	T& GetRef() { return GetRef_2(); }
	inline T& GetRef_1() {
		if (threadRepo.size() <= myIndex) {
			threadRepo.resize(myIndex + 1);
			threadRepo[myIndex] = defaultRecord;
		}
		return threadRepo[myIndex].value();
	}
	inline T& GetRef_2() {
		size_t size = threadRepo.size()-1;
		size_t testIndex = myIndex < size ? myIndex : size;
		if (!threadRepo[testIndex]) {
			if (threadRepo.size()-1 <= myIndex) {
				threadRepo.resize(myIndex + 2);
			}
			threadRepo[myIndex] = defaultRecord;
		}
		return threadRepo[myIndex].value();
	}

	const T& GetRef() const {
		size_t size = threadRepo.size() - 1;
		size_t testIndex = myIndex < size ? myIndex : size;
		if (!threadRepo[testIndex]) {
			if (threadRepo.size() - 1 <= myIndex) {
				threadRepo.resize(myIndex + 2);
			}
			threadRepo[myIndex] = defaultRecord;
		}
		return threadRepo[myIndex].value();
	}
private:
	size_t myIndex;

	static thread_local std::vector<std::optional<T>> threadRepo;
	static std::mutex allocatorLock;
	static SlabAllocatorEngine allocator;

	T defaultRecord;
};


template <class T>
thread_local std::vector<std::optional<T>> mi_tls<T>::threadRepo(1);

template <class T>
std::mutex mi_tls<T>::allocatorLock;

template <class T>
SlabAllocatorEngine mi_tls<T>::allocator(10);




} // namespace exc