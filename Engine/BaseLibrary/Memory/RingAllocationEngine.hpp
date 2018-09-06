#pragma once


#include <cstdint>
#include <limits>
#include <algorithm>
#include <vector>


namespace inl {


/// <summary>
/// This class provides a base for ring allocation tasks.
/// 
/// This class will not allocate the actual objects, it only
/// administrates the object positions and sizes.
///
/// </summary>
class RingAllocationEngine {
protected:
	enum class eCellState { FREE = 0, INSIDE, END, PREVIOUS_IN_USE };

	class CellContainer {
	public:
		CellContainer(size_t size);

		void Set(size_t index, eCellState value);

		eCellState At(size_t index) const;

		size_t Size() const;

		void Resize(size_t size);

		void Reset();

	protected:
		static constexpr int CELL_SIZE = 2;
		std::vector<bool> m_bitset;
	};

public:
	/// <summary>
	/// Initialize an allocator of specified size.
	/// </summary>
	/// <param name="poolSize">The number of available slots in the pool.</param>
	RingAllocationEngine(size_t poolSize);

	/// <summary> Allocates space from the pool for one item. </summary>
	/// <param name="allocationSize">(Optional) The size of the range that should be allocated.</param>
	/// <returns> The starting index of the allocated range. </returns>
	/// <exception cref="std::bad_alloc"> Thrown if allocation does not fit.</exception>
	/// <exception cref="std::invalid_argument">
	/// If allocation size is zero.
	/// </exception>
	size_t Allocate(size_t allocationSize = 1);

	/// <summary> Deallocated the range starting at index. </summary>
	/// <param name="index"> The starting index of the range that should be freed. </param>
	/// <exception cref="std::out_of_range"> Thrown if index is out of the pools range.</exception>
	void Deallocate(size_t index);

	/// <summary> Resizes the pool, allocated slots won't be cleared, but may become invalid if pool is shrunk. </summary>
	/// <param name="newPoolSize"> The number of available slots in the new pool. </param>
	void Resize(size_t newPoolSize);

	/// <summary>
	/// Clears all slots, does not affect pool size.
	/// Next allocation will be placed at the begginning.
	/// </summary>
	void Reset();

	size_t Size() const;

protected:
	size_t m_nextIndex;
	CellContainer m_container;
};


} // namespace inl

