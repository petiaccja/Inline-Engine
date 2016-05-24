#include "SameSizeAllocatorEngine.hpp"

#include "../BitOperations.hpp"


namespace exc {



SameSizeAllocatorEngine::SameSizeAllocatorEngine(size_t poolSize)
	: m_blocks((poolSize + SlotsPerBlock - 1)/ SlotsPerBlock)
{
	// chain all blocks into free list
	for (size_t idx = 0; idx < m_blocks.size(); ++idx) {
		m_blocks[idx].nextBlockIndex = idx + 1;
		m_blocks[idx].slotOccupancy = 0;
	}

	// set first and last blocks
	m_first = &m_blocks[0];
	m_last = &m_blocks[m_blocks.size() - 1];

	// mask out unused part of last block
	int numLastSlots = (poolSize % SlotsPerBlock);
	if (numLastSlots != 0) {
		m_last->slotOccupancy = ~size_t(0) << numLastSlots;
	}
}


size_t SameSizeAllocatorEngine::Allocate() {
	while (true) {
		while (prohibit) {}
		++firstRef;

		if (m_first == nullptr) {
			throw std::bad_alloc();
		}

		size_t mask = m_first->slotOccupancy;
		intptr_t index = CountTrailingZeros(~mask);

		if (index >= 0) { // there's free space
			// try to OR the bit we try to acquire with current mask
			size_t myBit = size_t(1) << index;
			size_t prev = m_first->slotOccupancy.fetch_or(myBit);
			if ((prev & myBit) == 0) { // bit was still free, so WE are the one who locked it
				// success...

				// get the acquired index
				size_t allocationIndex = IndexOf(m_first) * SlotsPerBlock + index;

				// the last one to succeed locking is responsible to get a new first
				if (prev != ~size_t(0) && (prev | myBit) == ~size_t(0)) {
					prohibit = true; // lock out everyone
					while (firstRef != 1) {} // wait until we are the only one locking first

					// replace first
					m_first = m_first->nextBlockIndex < m_blocks.size() ? &m_blocks[m_first->nextBlockIndex] : nullptr;

					// let in everyone again
					prohibit = false;
				}

				// let first be changable again
				--firstRef;

				return allocationIndex;
			}
		}

		// there was no free space || someone stole our choosen bit -> retry
		--firstRef;
	}
}


void SameSizeAllocatorEngine::Deallocate(size_t index) {
	// calculate block and slot index from supplied index
	Block* myBlock = &m_blocks[index / SlotsPerBlock];
	int myIndex = index % SlotsPerBlock;

	// mark my index as free slot
	size_t bitMask = ~(size_t(1) << myIndex);
	size_t prevSlotMask = myBlock->slotOccupancy.fetch_and(bitMask);

	// if block was full before our operation, add it to the free list
	if (prevSlotMask == ~size_t(0)) {
		// add it to the end of the list, to avoid fuckers deleting the current first after we added this as even more first
		m_last->nextBlockIndex = IndexOf(myBlock);
		m_last = myBlock;
		if (m_first == nullptr) {
			prohibit = true;
			while (firstRef != 0) {}
			m_first = m_last;
			prohibit = false;
		}
	}
}


} // namespace exc