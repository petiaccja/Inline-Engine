#include "Vertex.hpp"
#include <type_traits>
#include <iterator>


namespace inl {
namespace gxeng {



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
	iterator_impl operator++(int) { iterator tmp(*this); ++ptr; return tmp; }

	// bidirectional
	iterator_impl& operator--() { --ptr; return *this; }
	iterator_impl operator--(int) { iterator tmp(*this); --ptr; return tmp; }

	// random access
	iterator_impl& operator+=(difference_type n) { ptr += n; return *this; }
	iterator_impl& operator-=(difference_type n) { ptr -= n; return *this; }
	iterator_impl operator+(difference_type n) const { iterator tmp(*this); ptr += n; return tmp; }
	iterator_impl operator-(difference_type n) const { iterator tmp(*this); ptr -= n; return tmp; }
	difference_type operator-(const iterator_impl& rhs) const { return ptr - rhs.ptr; }

	bool operator<(const iterator_impl& rhs) const { return ptr < rhs.ptr; }
	bool operator>(const iterator_impl& rhs) const { return ptr > rhs.ptr; }
	bool operator<=(const iterator_impl& rhs) const { return ptr <= rhs.ptr; }
	bool operator>=(const iterator_impl& rhs) const { return ptr >= rhs.ptr; }
protected:
	T* ptr;
};


template <class T>
class HasIterator {
public:
	using type = typename std::remove_const<T>::type;
	class iterator : public iterator_impl<type> {
	public:
		template <class U>
		friend class const_iterator;
		using iterator_impl<type>::iterator_impl;
	};
};

class DoesNotHaveIterator {};


template <class T>
class const_iterator : public iterator_impl<const T> {
public:
	using iterator_impl<const T>::iterator_impl;
	const_iterator(HasIterator<T>::iterator& rhs) : ptr(rhs.ptr) {}
};



template <class ViewT>
class VertexArrayView : public std::conditional<std::is_const<ViewT>::value, HasIterator<ViewT>, DoesNotHaveIterator>::type {
	static constexpr bool IsConst = std::is_const<ViewT>::value;
public:
	using const_iterator = const_iterator<ViewT>;

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
		return *reinterpret_cast<ViewT*>(size_t(m_array) + idx*m_stride);
	}
	/// <summary> Access array by index. </summary>
	ViewT& operator[](size_t idx) const {
		assert(idx < m_size);
		return *reinterpret_cast<ViewT*>(size_t(m_array) + idx*m_stride);
	}


	void begin() {
		
	}


private:
	template <class PVertexT>
	void Init(PVertexT array, size_t size, size_t stride) {
		m_stride = stride;
		m_array = dynamic_cast<ViewT*>(array);
		m_size = size;
	}


	ViewT* m_array;
	size_t m_stride;
	size_t m_size;
};



}
}
