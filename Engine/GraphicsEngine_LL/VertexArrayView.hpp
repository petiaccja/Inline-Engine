#include "Vertex.hpp"
#include <type_traits>
#include <iterator>


namespace inl {
namespace gxeng {


template <class ViewT>
class VertexArrayView;


namespace impl {


template <class ViewT>
class HasIterator;


// Basic iterator class to iterate over VertexArrayViews.
template <class T>
class iterator_impl : std::iterator<std::random_access_iterator_tag, T> {
protected:
	iterator_impl(T* ptr) : ptr(ptr) {}
public:
	// ctor
	iterator_impl() : ptr(nullptr) {}
	iterator_impl(const iterator& rhs) : ptr(rhs.ptr) {}
	iterator_impl(iterator&& rhs) : ptr(rhs.ptr) {}

	// basic
	T& operator*() { return *ptr; }
	T* operator->() { return *ptr; }

	bool operator==(const iterator_impl& rhs) const { return ptr == rhs.ptr; }
	bool operator!=(const iterator_impl& rhs) const { return !(*this == rhs); }

	iterator_impl& operator++() { ++ptr; return *this; }
	iterator_impl operator++(int) { iterator_impl tmp(*this); ++ptr; return tmp; }

	// bidirectional
	iterator_impl& operator--() { --ptr; return *this; }
	iterator_impl operator--(int) { iterator_impl tmp(*this); --ptr; return tmp; }

	// random access
	iterator_impl& operator+=(difference_type n) { ptr += n; return *this; }
	iterator_impl& operator-=(difference_type n) { ptr -= n; return *this; }
	iterator_impl operator+(difference_type n) const { iterator_impl tmp(*this); ptr += n; return tmp; }
	iterator_impl operator-(difference_type n) const { iterator_impl tmp(*this); ptr -= n; return tmp; }
	difference_type operator-(const iterator_impl& rhs) const { return ptr - rhs.ptr; }

	bool operator<(const iterator_impl& rhs) const { return ptr < rhs.ptr; }
	bool operator>(const iterator_impl& rhs) const { return ptr > rhs.ptr; }
	bool operator<=(const iterator_impl& rhs) const { return ptr <= rhs.ptr; }
	bool operator>=(const iterator_impl& rhs) const { return ptr >= rhs.ptr; }
protected:
	T* ptr;
};


// Helper class to allow VertexArrayView to conditionally have non-const iterators.
template <class ViewT>
class HasIterator {
public:
	using type = typename std::remove_const<ViewT>::type;

	class iterator : public iterator_impl<type> {
		template <class ViewU> friend class VertexArrayView;
		template <class ViewU> friend class const_iterator;
	public:
		using iterator_impl<type>::iterator_impl;
	};
};


// Helper class to inherit from when not having non-const iterators.
class DoesNotHaveIterator {};


// Constant iterator that the view will always have.
template <class ViewT>
class const_iterator : public iterator_impl<const ViewT> {
	template <class ViewU> friend class VertexArrayView;
public:
	using iterator_impl<const ViewT>::iterator_impl;

	// Convert from mutable iterators.
	const_iterator(typename HasIterator<ViewT>::iterator& rhs) : ptr(rhs.ptr) {}
};


} // namespace impl


/// <summary>
/// Allows to edit or view a vertex array as if the vertices were another type.
/// Useful for safely iterating over VertexParts of a given vertex type.
/// </summary>
template <class ViewT>
class VertexArrayView : public std::conditional<!std::is_const<ViewT>::value, impl::HasIterator<ViewT>, impl::DoesNotHaveIterator>::type {
	static constexpr bool IsConst = std::is_const<ViewT>::value;
public:
	using const_iterator = impl::const_iterator<ViewT>;

	/// <summary> Create an empty view. </summary>
	VertexArrayView() : m_array(nullptr), m_stride(0), m_size(0) {}

	VertexArrayView(const VertexArrayView&) = default;

	VertexArrayView(VertexArrayView&&) = default;

	~VertexArrayView() = default;

	/// <summary> Create a view from a given array. </summary>
	/// <param name="array"> Pointer to the memory region you want to view and index. Must be castable to ViewT*. </summary>
	/// <param name="size"> Number of elements in the array. </summary>
	/// <param name="stride"> Size of an element in bytes. Typically sizeof(*array). </summary>
	/// <exception cref="std::bad_cast"> When given array cannot be dynamically cast to view type. </summary>
	template <class VertexT, class = std::enable_if_t<IsConst>>
	VertexArrayView(const VertexT* array, size_t size, size_t stride) {
		Init(array, size, stride);
	}

	/// <summary> Create a view from a given array. </summary>
	/// <param name="array"> Pointer to the memory region you want to view and index. Must be castable to ViewT*. </summary>
	/// <param name="size"> Number of elements in the array. </summary>
	/// <param name="stride"> Size of an element in bytes. Typically sizeof(*array). </summary>
	/// <exception cref="std::bad_cast"> When given array cannot be dynamically cast to view type. </summary>
	template <class VertexT, class = std::enable_if_t<!IsConst>>
	VertexArrayView(VertexT* array, size_t size, size_t stride) {
		static_assert(!std::is_const_v<VertexT>, "You cannot initalize a mutable view with a const array.");
		Init(array, size, stride);
	}



	/// <summary> Bind given array to view. </summary>
	/// <param name="array"> Pointer to the memory region you want to view and index. Must be castable to ViewT*. </summary>
	/// <param name="size"> Number of elements in the array. </summary>
	/// <param name="stride"> Size of an element in bytes. Typically sizeof(*array). </summary>
	/// <exception cref="std::bad_cast"> When given array cannot be dynamically cast to view type. </summary>
	template <class VertexT, class = std::enable_if_t<IsConst>>
	void Set(const VertexT* array, size_t size, size_t stride) {
		Init(array, size, stride);
	}

	/// <summary> Bind given array to view. </summary>
	/// <param name="array"> Pointer to the memory region you want to view and index. Must be castable to ViewT*. </summary>
	/// <param name="size"> Number of elements in the array. </summary>
	/// <param name="stride"> Size of an element in bytes. Typically sizeof(*array). </summary>
	/// <exception cref="std::bad_cast"> When given array cannot be dynamically cast to view type. </summary>
	template <class VertexT, class = std::enable_if_t<!IsConst>>
	void Set(VertexT* array, size_t size, size_t stride) {
		static_assert(!std::is_const_v<VertexT>, "You cannot initalize a mutable view with a const array.");
		Init(array, size, stride);
	}


	/// <summary> Unbind current array. </summary>
	void Clear() {
		m_array = nullptr;
		m_stride = m_size = 0;
	}

	// <summary> Get the number of elements. </summary>
	size_t Size() const {
		return m_size;
	}


	/// <summary> Access array by index. </summary>
	template<class = std::enable_if_t<!IsConst, const int>>
	ViewT& operator[](size_t idx) {
		assert(idx < m_size);
		return *GetOffsetedPointer(idx);
	}
	/// <summary> Access array by index. </summary>
	ViewT& operator[](size_t idx) const {
		assert(idx < m_size);
		return *GetOffsetedPointer(idx);
	}

	/// <summary> Get mutable iterator to the first element of the container. </summary>
	template <class = typename std::enable_if<!IsConst>::type>
	auto begin() {
		return iterator{ GetOffsetedPointer(0) };
	}
	/// <summary> Get mutable iterator to the end (past the last element) of the container. </summary>
	template <class = typename std::enable_if<!IsConst>::type>
	auto end() {
		return iterator{ GetOffsetedPointer(Size()) };
	}

	/// <summary> Get const iterator to the first element of the container. </summary>
	const_iterator begin() const {
		return const_iterator{ GetOffsetedPointer(0) };
	}
	/// <summary> Get const iterator to the end (past the last element) of the container. </summary>
	const_iterator end() const {
		return const_iterator{ GetOffsetedPointer(Size()) };
	}

	/// <summary> Get const iterator to the first element of the container. </summary>
	const_iterator cbegin() const {
		return const_iterator{ GetOffsetedPointer(0) };
	}
	/// <summary> Get const iterator to the end (past the last element) of the container. </summary>
	const_iterator cend() const {
		return const_iterator{ GetOffsetedPointer(Size()) };
	}

protected:
	// I don't honestly know if const overload is needed, but I don't really care.
	/// <summary> Return a ViewT* pointer to element given by index. </summary>
	ViewT* GetOffsetedPointer(size_t index) {
		return reinterpret_cast<ViewT*>(size_t(m_array) + index*m_stride);
	}
	/// <summary> Return a ViewT* pointer to element given by index. </summary>
	ViewT* GetOffsetedPointer(size_t index) const {
		return reinterpret_cast<ViewT*>(size_t(m_array) + index*m_stride);
	}

private:
	/// <summary> Init inner parameters and try to cast input to view type. </summary>
	/// <exception cref="std::bas_cast"> Tries to dynamic_cast input type to view type, which might throw bad_cast. </exception>
	template <class PVertexT>
	void Init(PVertexT array, size_t size, size_t stride) {
		m_stride = stride;
		m_array = dynamic_cast<ViewT*>(array);
		if (m_array == nullptr) {
			throw std::bad_cast();
		}
		m_size = size;
	}
private:
	ViewT* m_array;
	size_t m_stride;
	size_t m_size;
};



} // namespace gxeng
} // namespace inl
