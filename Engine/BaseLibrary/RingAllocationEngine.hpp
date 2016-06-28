#pragma once


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
/// </summary>
class RingAllocationEngine {
private:
	enum class eCellState { FREE = 0, INSIDE, END, PREVIOUS_IN_USE };

	class CellContainer {
	public:
		CellContainer(size_t size) :
		m_bitset(size*CELL_SIZE) {}

		void Set(size_t index, eCellState value) {
			bool first = int(value) >> 1;
			bool second = int(value) & 1;
			m_bitset[index*CELL_SIZE] = first;
			m_bitset[index*CELL_SIZE + 1] = second;
		}

		eCellState At(size_t index) const {
			uint8_t first = m_bitset[index*CELL_SIZE];
			uint8_t second = m_bitset[index*CELL_SIZE + 1];

			return eCellState((first<<1) + second);
		}

		size_t Size() const {
			return m_bitset.size() / CELL_SIZE;
		}

		void Resize(size_t size) {
			m_bitset.resize(size*CELL_SIZE);

		}

		void Reset() {
			size_t size = m_bitset.size();
			m_bitset.clear();
			m_bitset.resize(size);
		}

	protected:
		static constexpr int CELL_SIZE = 2;
		std::vector<bool> m_bitset;
	};

public:
	/// <summary>
	/// Initialize an allocator of specified size.
	/// </summary>
	/// <param name="poolSize">The number of available slots in the pool.</param>
	RingAllocationEngine(size_t poolSize) :
		m_container(poolSize),
		m_nextIndex(0) {}


	/// <summary> Allocates space from the pool for one item. </summary>
	/// <param name="allocationSize">(Optional) The size of the range that should be allocated.</param>
	/// <returns> The starting index of the allocated range. </returns>
	/// <exception cref="std::bad_alloc"> Thrown if allocation does not fit.</exception>
	/// <exception cref="std::invalid_argument">
	/// If allocation size is zero.
	/// </exception>
	size_t Allocate(size_t allocationSize = 1) {
		if (allocationSize > GetPoolSize()) {
			throw std::bad_alloc();
		}
		if (allocationSize == 0) {
			throw std::invalid_argument("Allocation size should be non-zero.");
		}

		size_t allocStartIndex = m_nextIndex;
		if (allocStartIndex + allocationSize > GetPoolSize()) {
			allocStartIndex = 0;
		}

		// check if last cell of allocation is free
		// if it is, then the whole range of the allocation is free
		{
			bool isLastFree = m_container.At(allocStartIndex + allocationSize-1) == eCellState::FREE;
			if (!isLastFree) {
				throw std::bad_alloc();
			}
		}

		// mark allocated area
		{
			size_t end = allocationSize-1;
			for (size_t i = 0; i < end; i++) {
				m_container.Set(allocStartIndex + i, eCellState::INSIDE);
			}
			m_container.Set(allocStartIndex + end, eCellState::END);
		}


		m_nextIndex = (allocStartIndex + allocationSize) % m_container.Size();

		return allocStartIndex;
	}


	/// <summary> Deallocated the range starting at index. </summary>
	/// <param name="index"> The starting index of the range that should be freed. </param>
	/// <exception cref="std::out_of_range"> Thrown if index is out of the pools range.</exception>
	void Deallocate(size_t index) {
		if (index >= m_container.Size()) {
			throw std::out_of_range("Given index is greater than the highest index in the pool");
		}

		static_assert(std::is_unsigned<size_t>::value, "Utilizing underflow. Size type should be unsigned.");
		size_t prevIndex = index-1;
		prevIndex = prevIndex >= m_container.Size() ? m_container.Size()-1 : prevIndex;
		bool isPreviousInUse = m_container.At(prevIndex) != eCellState::FREE;
		bool previousIsNotFront = index != m_nextIndex;

		if (previousIsNotFront && isPreviousInUse) {
			// there are still allocated blocks before this one

			// mark all cells unused inside this allocation
			size_t current = index;
			bool isEnd;
			do {
				isEnd = m_container.At(current) == eCellState::END;
				m_container.Set(current, eCellState::PREVIOUS_IN_USE);
				current = (current+1) % m_container.Size();
			} while(!isEnd);
		}
		else {
			// if it has no allocated space befor this one
			// in other words if this is the last allocated range, it is time to deallocate

			size_t current = index;

			// lets free this allocation first
			{
				bool isEnd;
				do {
					isEnd = m_container.At(current) == eCellState::END;
					m_container.Set(current, eCellState::FREE);
					current = (current+1) % m_container.Size();
				} while(!isEnd);
			}

			// go forward, and free every cell in a contigous range starting at this cell
			// that is in the state of "previous in use"
			{
				while (m_container.At(current) == eCellState::PREVIOUS_IN_USE) {
					m_container.Set(current, eCellState::FREE);
					current = (current+1) % m_container.Size();
				}
			}
		}
	}


	/// <summary> Resizes the pool, allocated slots won't be cleared, but may become invalid if pool is shrunk. </summary>
	/// <param name="newPoolSize"> The number of available slots in the new pool. </param>
	void Resize(size_t newPoolSize) {
		m_container.Resize(newPoolSize);
		if (m_nextIndex >= m_container.Size()) {
			m_nextIndex = 0;
		}
	}


	/// <summary> Clears all slots, does not affect pool size. </summary>
	void Reset() {
		m_container.Reset();
		m_nextIndex = 0;
	}


	size_t GetPoolSize() const {
		return m_container.Size();
	}


protected:
	size_t m_nextIndex;
	CellContainer m_container;
};


} // namespace exc

