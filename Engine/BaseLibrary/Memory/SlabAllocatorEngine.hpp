#pragma once

#include <cstddef>
#include <vector>


namespace exc {


/// <summary>
/// Serves as a base for allocators that allocate object of the same size.
/// Maintains a list of slots, each of which fits an object. Allocating
/// an object returns the index of the next free slot. This class does
/// NOT handle space allocation for the object, only slot allocation.
/// Space allocation is up to the user from his own pool.
/// </summary>
class SlabAllocatorEngine {
	// How it works:
	// Slots are grouped into blocks of size_t slots.
	// There's an array of such blocks, each block containing a bit mask that indicate
	// if those slots are free.
	// Block that contain at least one free slot are linked into a list. Allocation
	// check the free list, populates a slot and pops the first block if it's full.
private:
	struct Block {
		Block() = default;
		Block(size_t nextBlockIndex, size_t slotOccupancy) : nextBlockIndex(nextBlockIndex), slotOccupancy(slotOccupancy) {}
		size_t nextBlockIndex; /// <summary> Index of the next block in the free-list. </summary>
		size_t slotOccupancy; /// <summary> 0 means the slot if free, 1 is occupied. </summary>
	};
	static constexpr unsigned SlotsPerBlock = 8 * sizeof(size_t);
public:
	/// <summary>
	/// Initialize an allocator of specified size.
	/// </summary>
	/// <param name="poolSize">The number of available slots in the pool.</param>
	SlabAllocatorEngine(size_t poolSize);
	SlabAllocatorEngine(const SlabAllocatorEngine& rhs);
	SlabAllocatorEngine(SlabAllocatorEngine&& rhs);

	SlabAllocatorEngine& operator=(const SlabAllocatorEngine& rhs);
	SlabAllocatorEngine& operator=(SlabAllocatorEngine&& rhs);

	/// <summary> Allocates space from the pool for one item. </summary>
	/// <returns> The index of the allocated slot. </returns>
	/// <exception cref="std::bad_alloc"> Thrown if pool is full. </exception>
	size_t Allocate();

	/// <summary> Deallocated the slot specified by the index. </summary>
	void Deallocate(size_t index);

	/// <summary> Resizes the pool, allocated slots won't be cleared, but may become invalid if pool is shrunk. </summary>
	/// <param name="newPoolSize"> The number of available slots in the new pool. </param>
	void Resize(size_t newPoolSize);

	/// <summary> Clears all slots, does not affect pool size. </summary>
	void Reset();


	/// <summary> Get the total number of slots (free + taken). </summary>
	size_t Size() const { return m_poolSize; }
private:
	inline size_t IndexOf(const Block* block) const {
		return block - m_blocks.data();
	}
	inline void BlockOf(size_t globalIndex, Block*& block, int& inBlockIndex) {
		size_t divRes = globalIndex / SlotsPerBlock;
		block = &m_blocks[divRes];
		inBlockIndex = globalIndex - (divRes * SlotsPerBlock); // globalIndex % SlotsPerBlock costs much more
	}
private:
	size_t m_poolSize;
	std::vector<Block> m_blocks;
	Block* m_first;
};


} // namespace exc