#include <type_traits>
#include <iterator>


namespace inl {


template <class ViewT>
class ArrayView;


namespace impl {


template <class ViewT>
class HasIterator;


// Basic iterator class to iterate over ArrayViews.
template <class T>
class iterator_impl {
protected:
	iterator_impl(T* ptr) : ptr(ptr) {}
public:
	using value_type = T;
	using difference_type = ptrdiff_t;
	using pointer = T*;
	using reference = std::decay_t<T>&;
	using iterator_category = std::random_access_iterator_tag;

	// ctor
	iterator_impl() : ptr(nullptr) {}
	iterator_impl(const iterator_impl& rhs) : ptr(rhs.ptr) {}
	iterator_impl(iterator_impl&& rhs) : ptr(rhs.ptr) {}

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


// Helper class to allow ArrayView to conditionally have non-const iterators.
template <class ViewT>
class HasIterator {
public:
	using type = typename std::remove_const<ViewT>::type;

	class iterator : public iterator_impl<type> {
		template <class ViewU> friend class ArrayView;
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
	template <class ViewU> friend class ArrayView;
public:
	using iterator_impl<const ViewT>::iterator_impl;

	// Convert from mutable iterators.
	const_iterator(typename HasIterator<ViewT>::iterator& rhs) : iterator_impl(rhs.ptr) {}
};


} // namespace impl


/// <summary>
/// Allows to iterate over a typeless memory region as if it were an array of certain type.
/// </summary>
template <class ViewT>
class ArrayView : public std::conditional<!std::is_const<ViewT>::value, impl::HasIterator<ViewT>, impl::DoesNotHaveIterator>::type {
	static constexpr bool IsConst = std::is_const<ViewT>::value;
public:
	using const_iterator = impl::const_iterator<ViewT>;

	/// <summary> Create an empty view. </summary>
	ArrayView() : m_view(nullptr), m_stride(0), m_size(0) {}

	ArrayView(const ArrayView&) = default;

	ArrayView(ArrayView&&) = default;

	~ArrayView() = default;

	/// <summary> Create a view from a given array. </summary>
	/// <param name="array"> Pointer to the memory region you want to view and index. Must be castable to ViewT*. </summary>
	/// <param name="size"> Number of elements in the array. </summary>
	/// <param name="stride"> Size of an element in bytes. Typically sizeof(*array). </summary>
	/// <exception cref="std::bad_cast"> When given array cannot be dynamically cast to view type. </summary>
	template <class ArrayT, class = std::enable_if_t<IsConst>>
	ArrayView(const ArrayT* array, size_t size, size_t stride) {
		Init(array, size, stride);
	}

	/// <summary> Create a view from a given array. </summary>
	/// <param name="array"> Pointer to the memory region you want to view and index. Must be castable to ViewT*. </summary>
	/// <param name="size"> Number of elements in the array. </summary>
	/// <param name="stride"> Size of an element in bytes. Typically sizeof(*array). </summary>
	/// <exception cref="std::bad_cast"> When given array cannot be dynamically cast to view type. </summary>
	template <class ArrayT, class = std::enable_if_t<!IsConst>>
	ArrayView(ArrayT* array, size_t size, size_t stride) {
		static_assert(!std::is_const_v<ArrayT>, "You cannot initalize a mutable view with a const array.");
		Init(array, size, stride);
	}



	/// <summary> Bind given array to view. </summary>
	/// <param name="array"> Pointer to the memory region you want to view and index. Must be castable to ViewT*. </summary>
	/// <param name="size"> Number of elements in the array. </summary>
	/// <param name="stride"> Size of an element in bytes. Typically sizeof(*array). </summary>
	/// <exception cref="std::bad_cast"> When given array cannot be dynamically cast to view type. </summary>
	template <class ArrayT, class = std::enable_if_t<IsConst>>
	void Set(const ArrayT* array, size_t size, size_t stride) {
		Init(array, size, stride);
	}

	/// <summary> Bind given array to view. </summary>
	/// <param name="array"> Pointer to the memory region you want to view and index. Must be castable to ViewT*. </summary>
	/// <param name="size"> Number of elements in the array. </summary>
	/// <param name="stride"> Size of an element in bytes. Typically sizeof(*array). </summary>
	/// <exception cref="std::bad_cast"> When given array cannot be dynamically cast to view type. </summary>
	template <class ArrayT, class = std::enable_if_t<!IsConst>>
	void Set(ArrayT* array, size_t size, size_t stride) {
		static_assert(!std::is_const_v<ArrayT>, "You cannot initalize a mutable view with a const array.");
		Init(array, size, stride);
	}


	/// <summary> Unbind current array. </summary>
	void Clear() {
		m_view = nullptr;
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
	template <class Q = typename impl::HasIterator<ViewT>::iterator>
	Q begin() {
		return impl::HasIterator<ViewT>::iterator{ GetOffsetedPointer(0) };
	}
	/// <summary> Get mutable iterator to the end (past the last element) of the container. </summary>
	template <class Q = typename impl::HasIterator<ViewT>::iterator>
	Q end() {
		return impl::HasIterator<ViewT>::iterator{ GetOffsetedPointer(Size()) };
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
		return reinterpret_cast<ViewT*>(size_t(m_view) + index*m_stride);
	}
	/// <summary> Return a ViewT* pointer to element given by index. </summary>
	ViewT* GetOffsetedPointer(size_t index) const {
		return reinterpret_cast<ViewT*>(size_t(m_view) + index*m_stride);
	}

private:
	template <class T, class U>
	U Cast(std::enable_if_t<std::is_polymorphic<T>::value, T> t) {
		return dynamic_cast<U>(t);
	}
	template <class T, class U>
	U Cast(std::enable_if_t<!std::is_polymorphic<T>::value && (std::is_base_of<U, T>::value || std::is_base_of<T, U>::value), T> t) {
		return static_cast<U>(t);
	}
	template <class T, class U>
	U Cast(std::enable_if_t<!std::is_polymorphic<T>::value && !std::is_base_of<U, T>::value && !std::is_base_of<T, U>::value, T> t) {
		return reinterpret_cast<U>(t);
	}

	/// <summary> Init inner parameters and try to cast input to view type. </summary>
	/// <remarks> Tries dynamic_cast, static_cast and reinterpret_cast in this order. </remarks>
	template <class PArrayT>
	void Init(PArrayT array, size_t size, size_t stride) {
		m_stride = stride;
		m_view = Cast<ViewT*, PArrayT>(array);
		m_size = size;
	}
private:
	ViewT* m_view;
	size_t m_stride;
	size_t m_size;
};


} // namespace inl
