#pragma once

#if 0

#include <cstdint>
#include <limits>
#include <algorithm>
#include <vector>


namespace exc {

/// <summary>
/// This class provides a base for ring allocation tasks.
/// 
/// This class will not allocate the actual objects, it only
/// administrates the object positions and sizes.
///
/// SizeType will be used as the type of indices, and allocation sizes.
/// Choosing a type for SizeType that occupies less space can increase performance.
/// Please note that no allocation of size equal to the maximum value of SizeType can be made.
/// </summary>
using SizeType = size_t; //TODO cahnge to template parameter
class RingAllocationEngine {
protected:
	struct CellData {
		SizeType allocSize;
		bool inUse; // Indicates weather this allocation is still needed

		CellData() : allocSize(0), inUse(false) {}
	};

public:
	static_assert(std::is_unsigned<SizeType>::value, "SizeType should be unsigned.");

	/// <summary>
	/// Initialize an allocator of specified size.
	/// </summary>
	/// <param name="poolSize">The number of available slots in the pool.</param>
	RingAllocationEngine(SizeType poolSize)
		: m_pool(poolSize) {
	}


	/// <summary> Allocates space from the pool for one item. </summary>
	/// <param name="allocationSize">(Optional) The size of the range that should be allocated.</param>
	/// <returns> The starting index of the allocated range. </returns>
	/// <exception cref="std::bad_alloc"> Thrown if allocation does not fit.</exception>
	/// <exception cref="std::invalid_argument">
	/// Thrown if allocation size is equal to the maximum value of SizeType.
	/// Or if allocation size is zero.
	/// </exception>
	SizeType Allocate(SizeType allocationSize = 1) {
		if (allocationSize > m_pool.size()) {
			throw std::bad_alloc();
		}
		if (allocationSize == 0 || allocationSize == INSIDE_ALLOCATION_ID) {
			throw std::invalid_argument("Allocation size should be non-zero, and less than maximum representable value.");
		}

		SizeType allocStartIndex = m_nextIndex;
		if (allocStartIndex + allocationSize > m_pool.size()) {
			allocStartIndex = 0;
		}

		bool isLastFree = m_pool[allocStartIndex + allocationSize-1].allocSize == 0;
		if (!isLastFree) {
			throw std::bad_alloc();
		}

		m_pool[allocStartIndex].allocSize = allocationSize;
		m_pool[allocStartIndex].inUse = true;
		for (SizeType i = 1; i < allocationSize; i++) {
			m_pool[allocStartIndex + i].allocSize = INSIDE_ALLOCATION_ID;
		}

		m_nextIndex = (allocStartIndex + allocationSize) % m_pool.size();

		return allocStartIndex;
	}


	// TODO calling this function in a particular order can create a pattern that makes the allocation alocate over an already allocated range

	/// <summary> Deallocated the range starting at index. </summary>
	/// <param name="index"> The starting index of the range that should be freed. </param>
	/// <exception cref="std::out_of_range"> Thrown if index is out of the pools range.</exception>
	void Deallocate(SizeType index) {
		if (index >= m_pool.size()) {
			throw std::out_of_range("Given index is greater than the highest index in the pool");
		}

		SizeType allocatedSize = m_pool[index].allocSize;
		SizeType prevIndex = index-1;
		prevIndex = prevIndex >= m_pool.size() ? m_pool.size()-1 : prevIndex;
		bool isPreviousAllocated = m_pool[prevIndex].allocSize != 0;
		bool hasGapBetweenFrontAndThis = index != m_nextIndex;
		if (hasGapBetweenFrontAndThis && isPreviousAllocated) {
			//just mark unused, but keep allocation, because there are still allocated blocks before this one
			m_pool[index].inUse = false;
		} else {
			//if it has no allocated space befor this one
			//in other words if this is the last allocated range, it is time to deallocate

			//find the last allocation starting from here, that is not in use (can be freed)
			SizeType lastFreeableIndex = index;
			do {
				//TODO
				static_assert(false, "TODO");
			} while ();

		}

		memset(m_pool.data(), 0, allocatedSize * sizeof(SizeType));
	}


	/// <summary> Resizes the pool, allocated slots won't be cleared, but may become invalid if pool is shrunk. </summary>
	/// <param name="newPoolSize"> The number of available slots in the new pool. </param>
	void Resize(SizeType newPoolSize) {
		m_pool.resize(newPoolSize);
	}


	/// <summary> Clears all slots, does not affect pool size. </summary>
	void Reset() {
		memset(m_pool.data(), 0, sizeof(SizeType) * m_pool.size());
	}
	
protected:
	static constexpr SizeType INSIDE_ALLOCATION_ID = std::numeric_limits<SizeType>::max();

	std::vector<CellData> m_pool;
	SizeType m_nextIndex;

};


} // namespace exc

#endif //0
