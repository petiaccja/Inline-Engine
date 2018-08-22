#pragma once

#include "SlabAllocatorEngine.hpp"

#include <map>
#include <algorithm>
#include <mutex>
#include <set>
#include <deque>
#include <optional>

namespace inl {


template <class T>
class mi_tls {
public:
	mi_tls() {
		AllocateSlot();
	}

	~mi_tls() {
		// deallocate our slot
		std::lock_guard<std::mutex> lkg(indexAllocatorLock);

		indexAllocator.Deallocate(myIndex);
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
		std::lock_guard<std::mutex> lkg(indexAllocatorLock);

		try {
			myIndex = indexAllocator.Allocate();
		}
		catch (std::bad_alloc&) {
			size_t currentSize = indexAllocator.Size();
			indexAllocator.Resize(size_t(currentSize * 1.2 + 1));
			myIndex = indexAllocator.Allocate(); // supposed to have enough space now
		}
	}


	T& GetRef() {
		size_t size = threadObjects.size()-1;
		size_t testIndex = myIndex < size ? myIndex : size;
		if (!threadObjects[testIndex]) {
			if (threadObjects.size()-1 <= myIndex) {
				threadObjects.resize(myIndex + 2);
			}
			threadObjects[myIndex] = defaultRecord;
		}
		return threadObjects[myIndex].value();
	}

	const T& GetRef() const {
		size_t size = threadObjects.size() - 1;
		size_t testIndex = myIndex < size ? myIndex : size;
		if (!threadObjects[testIndex]) {
			if (threadObjects.size() - 1 <= myIndex) {
				threadObjects.resize(myIndex + 2);
			}
			threadObjects[myIndex] = defaultRecord;
		}
		return threadObjects[myIndex].value();
	}

	//void UnregisterByThread() {
	//	std::lock_guard<std::mutex> lkg(registryLock);
	//	threadObjects.clear();
	//	for (auto& instance : threadInstances) {
	//		instance->myThreadObjects.erase(&threadObjects);
	//		instant->myThreadInstances.erase(&threadInstances);
	//	}
	//}

	//void UnregisterByInstance() {
	//	std::lock_guard<std::mutex> lkg(registryLock);
	//	for (auto& instances : myThreadInstances) {
	//		instances.erase(this);
	//	}
	//	for (auto& objects : myThreadObjects) {
	//		objects[myIndex].erase()
	//	}
	//}
private:
	static thread_local std::deque<std::optional<T>> threadObjects; // vector would be better, but deque.resize does not invalidate refs and ptrs
	static std::mutex indexAllocatorLock;
	static SlabAllocatorEngine indexAllocator;

	size_t myIndex;
	T defaultRecord;

	//static std::mutex registryLock;
	//thread_local std::set<mi_tls*> threadInstances;
	//std::set<decltype(threadObjects)*> myThreadObjects;
	//std::set<decltype(threadInstances)> myThreadInstances;
};


template <class T>
thread_local std::deque<std::optional<T>> mi_tls<T>::threadObjects(1);

template <class T>
std::mutex mi_tls<T>::indexAllocatorLock;

template <class T>
SlabAllocatorEngine mi_tls<T>::indexAllocator(10);




} // namespace inl
