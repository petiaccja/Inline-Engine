#include <new>
#include <unordered_map>
#include <malloc.h>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include "StackTrace.hpp"


template <class T>
struct MallocAllocator {
	typedef T value_type;

	MallocAllocator() = default;
	template <class U> MallocAllocator(const MallocAllocator<U>&) {}

	T* allocate(std::size_t n) { 
		return static_cast<T*>(std::malloc(n * sizeof(T))); 
	}

	void deallocate(T* p, std::size_t) { 
		std::free(p);
	}
};

template <class T, class U>
bool operator==(const MallocAllocator<T>&, const MallocAllocator<U>&) { 
	return true;
}

template <class T, class U>
bool operator!=(const MallocAllocator<T>&, const MallocAllocator<U>&) { 
	return false; 
}



struct AllocationInfo {
	std::vector<StackFrame<MallocAllocator>, MallocAllocator<StackFrame<MallocAllocator>>> stackTrace;
};



class AllocationLibrary {
public:
	static AllocationLibrary* GetInstance() {
		static AllocationLibrary instance;
		return &instance;
	}

	~AllocationLibrary() {
		std::cout << "Total number of allocations: " << count << std::endl;
		std::cout << "Number of memory leaks: " << allocations.size() << std::endl;
		alive = false;
	}

	std::mutex mtx;
	std::unordered_map<void*, AllocationInfo, std::hash<void*>, std::equal_to<void*>, MallocAllocator<std::pair<void*, AllocationInfo>>> allocations;
	bool alive = true;
	uint64_t count = 0;
};


void* TrackedAlloc(size_t size) {
	AllocationLibrary* store = AllocationLibrary::GetInstance();

	void* ptr = malloc(size);
	if (ptr == nullptr) {
		throw std::bad_alloc();
	}

	if (store->alive) {
		std::lock_guard<std::mutex>(store->mtx);

		AllocationInfo info;
		info.stackTrace = GetStackTrace<MallocAllocator>();
		store->allocations.insert_or_assign(ptr, std::move(info));
		store->count++;
	}

	return ptr;
}

void TrackedFree(void* ptr) {
	AllocationLibrary* store = AllocationLibrary::GetInstance();

	if (store->alive) {
		std::lock_guard<std::mutex>(store->mtx);

		store->allocations.erase(ptr);
	}

	free(ptr);
}


// Override new
void* operator new(size_t size) {
	return TrackedAlloc(size);
}


// Overload delete
void operator delete(void* ptr) {
	return TrackedFree(ptr);
}

// Override new[]
void* operator new[](size_t size) {
	return TrackedAlloc(size);
}


// Overload delete[]
void operator delete[](void* ptr) {
	return TrackedFree(ptr);
}