#pragma once


#include <vector>
#include <atomic>
#include <mutex>


namespace exc {



/// <summary>
/// Serves as a base for allocators that allocate object of the same size.
/// Maintains a list of slots, each of which fits an object. Allocating
/// an object returns the index of the next free slot. This class does
/// NOT handle space allocation for the object, only slot allocation.
/// Space allocation is up to the user from his own pool.
/// </summary>
class SameSizeAllocatorEngine {
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
		std::atomic<size_t> slotOccupancy; /// <summary> 0 means the slot if free, 1 is occupied. </summary>
	};
	static constexpr unsigned SlotsPerBlock = 8 * sizeof(size_t);
public:
	SameSizeAllocatorEngine(size_t poolSize);

	size_t Allocate();
	void Deallocate(size_t index);

private:
	inline size_t IndexOf(const Block* block) const {
		return block - &m_blocks[0];
	}
private:
	std::vector<Block> m_blocks;
	Block* m_first;
	Block* m_last;
	std::atomic<int> firstRef = 0;
	std::atomic<bool> prohibit = false;
	std::mutex m_listLock;
};



}; // namespace exc