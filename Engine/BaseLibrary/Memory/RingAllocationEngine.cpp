#include "RingAllocationEngine.hpp"

#include "../Exception/Exception.hpp"

#include <cassert>

namespace exc {

RingAllocationEngine::CellContainer::CellContainer(size_t size) :
	m_bitset(size*CELL_SIZE)
{}


void RingAllocationEngine::CellContainer::Set(size_t index, eCellState value) {
	bool first = (int(value) >> 1) != 0;
	bool second = (int(value) & 1) != 0;
	m_bitset[index*CELL_SIZE] = first;
	m_bitset[index*CELL_SIZE + 1] = second;
}


RingAllocationEngine::eCellState RingAllocationEngine::CellContainer::At(size_t index) const {
	uint8_t first = m_bitset[index*CELL_SIZE];
	uint8_t second = m_bitset[index*CELL_SIZE + 1];

	return eCellState((first<<1) + second);
}


size_t RingAllocationEngine::CellContainer::Size() const {
	return m_bitset.size() / CELL_SIZE;
}


void RingAllocationEngine::CellContainer::Resize(size_t size) {
	m_bitset.resize(size*CELL_SIZE);

}


void RingAllocationEngine::CellContainer::Reset() {
	size_t size = m_bitset.size();
	m_bitset.clear();
	m_bitset.resize(size);
}




RingAllocationEngine::RingAllocationEngine(size_t poolSize) :
	m_container(poolSize),
	m_nextIndex(0)
{}



size_t RingAllocationEngine::Allocate(size_t allocationSize) {
	if (allocationSize > Size()) {
		throw std::bad_alloc();
	}
	if (allocationSize == 0) {
		throw InvalidArgumentException("Allocation size should be non-zero.");
	}

	size_t allocStartIndex = m_nextIndex;
	if (allocStartIndex + allocationSize > Size()) {
		allocStartIndex = 0;
	}

	// check if first and last cell of allocation is free
	// if they are, then the whole range of the allocation is free
	{
		bool lastFree = m_container.At(allocStartIndex + allocationSize-1) == eCellState::FREE;
		bool firstFree = m_container.At(allocStartIndex) == eCellState::FREE;
		if (!lastFree || !firstFree) {
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


void RingAllocationEngine::Deallocate(size_t index) {
	if (index >= m_container.Size()) {
		throw OutOfRangeException("Given index is greater than the highest index in the pool");
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



void RingAllocationEngine::Resize(size_t newPoolSize) {
	m_container.Resize(newPoolSize);
	if (m_nextIndex >= m_container.Size()) {
		m_nextIndex = 0;
	}
}


void RingAllocationEngine::Reset() {
	m_container.Reset();
	m_nextIndex = 0;
}


size_t RingAllocationEngine::Size() const {
	return m_container.Size();
}


} // namespace exc
