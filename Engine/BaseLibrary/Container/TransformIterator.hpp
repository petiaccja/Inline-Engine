#pragma once

#include <iterator>
#include <type_traits>


namespace inl {
namespace impl {
	template <class Iterator, class Transformer>
	class TransformIteratorBase {
	public:
		using value_type = std::remove_reference_t<decltype(std::declval<Transformer>()(*std::declval<Iterator>()))>;
		using reference = value_type&;
		using pointer = value_type*;
		using iterator_category = typename Iterator::iterator_category;

		TransformIteratorBase() = default;
		explicit TransformIteratorBase(Iterator base);
		TransformIteratorBase(const TransformIteratorBase&) = default;
		TransformIteratorBase& operator=(const TransformIteratorBase&) = default;

		reference operator*() const;
		TransformIteratorBase& operator++();
		TransformIteratorBase operator++(int);

	protected:
		template <class T>
		decltype(auto) transform(T&& arg) const;

	protected:
		Iterator base;
	};


	template <class Iterator, class Transformer>
	class TransformInputIterator : protected TransformIteratorBase<Iterator, Transformer> {
		using Base = TransformIteratorBase<Iterator, Transformer>;

	public:
		using Base::iterator_category;
		using Base::pointer;
		using Base::reference;
		using Base::value_type;

		using Base::operator*;

		TransformInputIterator() = default;
		explicit TransformInputIterator(Iterator base);
		TransformInputIterator(const TransformInputIterator&) = default;
		TransformInputIterator& operator=(const TransformInputIterator&) = default;

		pointer operator->() const;
		TransformInputIterator& operator++();
		TransformInputIterator operator++(int);

		bool operator==(const TransformInputIterator& rhs);
		bool operator!=(const TransformInputIterator& rhs);
	protected:
		using Base::base;
	};


	template <class Iterator, class Transformer>
	class TransformForwardIterator : protected TransformInputIterator<Iterator, Transformer> {
		using Base = TransformInputIterator<Iterator, Transformer>;

	public:
		using Base::iterator_category;
		using Base::pointer;
		using Base::reference;
		using Base::value_type;

		using Base::operator*;
		using Base::operator->;

		TransformForwardIterator() = default;
		explicit TransformForwardIterator(Iterator base);
		TransformForwardIterator(const TransformForwardIterator&) = default;
		TransformForwardIterator& operator=(const TransformForwardIterator&) = default;

		TransformForwardIterator& operator++();
		TransformForwardIterator operator++(int);

		bool operator==(const TransformForwardIterator& rhs);
		bool operator!=(const TransformForwardIterator& rhs);
	protected:
		using Base::base;
	};


	template <class Iterator, class Transformer>
	class TransformBidirectionalIterator : protected TransformForwardIterator<Iterator, Transformer> {
		using Base = TransformForwardIterator<Iterator, Transformer>;

	public:
		using Base::iterator_category;
		using Base::pointer;
		using Base::reference;
		using Base::value_type;

		using Base::operator*;
		using Base::operator->;

		TransformBidirectionalIterator() = default;
		explicit TransformBidirectionalIterator(Iterator base);
		TransformBidirectionalIterator(const TransformBidirectionalIterator&) = default;
		TransformBidirectionalIterator& operator=(const TransformBidirectionalIterator&) = default;

		TransformBidirectionalIterator& operator++();
		TransformBidirectionalIterator operator++(int);
		TransformBidirectionalIterator& operator--();
		TransformBidirectionalIterator operator--(int);

		bool operator==(const TransformBidirectionalIterator& rhs);
		bool operator!=(const TransformBidirectionalIterator& rhs);
	protected:
		using Base::base;
	};

	struct Checker {
		int operator()(int arg) const { return arg; }
	};

	template <class Iterator, class Transformer>
	class TransformRandomIterator : protected TransformBidirectionalIterator<Iterator, Transformer> {
		using Base = TransformBidirectionalIterator<Iterator, Transformer>;

	public:
		using Base::iterator_category;
		using Base::pointer;
		using Base::reference;
		using Base::value_type;
		using difference_type = typename Iterator::difference_type;

		using Base::operator*;
		using Base::operator->;

		TransformRandomIterator() = default;
		explicit TransformRandomIterator(Iterator base);
		TransformRandomIterator(const TransformRandomIterator&) = default;
		TransformRandomIterator& operator=(const TransformRandomIterator&) = default;

		TransformRandomIterator& operator++();
		TransformRandomIterator operator++(int);
		TransformRandomIterator& operator--();
		TransformRandomIterator operator--(int);

		TransformRandomIterator& operator+=(difference_type n);
		TransformRandomIterator friend operator+(difference_type n, const TransformRandomIterator& i);
		TransformRandomIterator operator+(difference_type n) const;
		TransformRandomIterator& operator-=(difference_type n);
		TransformRandomIterator operator-(difference_type n) const;
		difference_type operator-(const TransformRandomIterator& i) const;
		reference operator[](difference_type n) const;

		bool operator==(const TransformRandomIterator& rhs);
		bool operator!=(const TransformRandomIterator& rhs);
		bool operator<(const TransformRandomIterator& rhs) const;
		bool operator>(const TransformRandomIterator& rhs) const;
		bool operator<=(const TransformRandomIterator& rhs) const;
		bool operator>=(const TransformRandomIterator& rhs) const;
	protected:
		using Base::base;
	};


	// They are the same, except for the iterator_category.
	template <class Iterator, class Transformer>
	using TransformContiguousIterator = TransformRandomIterator<Iterator, Transformer>;

	template <class Iterator, class Transformer>
	using SelectBase = std::conditional_t<std::is_same_v<typename Iterator::iterator_category, std::contiguous_iterator_tag>, TransformContiguousIterator<Iterator, Transformer>,
										  std::conditional_t<std::is_same_v<typename Iterator::iterator_category, std::random_access_iterator_tag>, TransformRandomIterator<Iterator, Transformer>,
															 std::conditional_t<std::is_same_v<typename Iterator::iterator_category, std::bidirectional_iterator_tag>, TransformBidirectionalIterator<Iterator, Transformer>,
																				std::conditional_t<std::is_same_v<typename Iterator::iterator_category, std::forward_iterator_tag>, TransformForwardIterator<Iterator, Transformer>,
																								   std::conditional_t<std::is_same_v<typename Iterator::iterator_category, std::input_iterator_tag>, TransformInputIterator<Iterator, Transformer>,
																													  TransformIteratorBase<Iterator, Transformer>>>>>>;

} // namespace impl



template <class Iterator, class Transformer>
class TransformIterator : public impl::SelectBase<Iterator, Transformer> {
	using impl::SelectBase<Iterator, Transformer>::SelectBase;
};


namespace impl {
	//------------------------------------------------------------------------------
	// Basic iterator.
	//------------------------------------------------------------------------------
	template <class Iterator, class Transformer>
	TransformIteratorBase<Iterator, Transformer>::TransformIteratorBase(Iterator base)
		: base(base) {}

	template <class Iterator, class Transformer>
	typename TransformIteratorBase<Iterator, Transformer>::reference TransformIteratorBase<Iterator, Transformer>::operator*() const {
		return transform(*base);
	}

	template <class Iterator, class Transformer>
	TransformIteratorBase<Iterator, Transformer>& TransformIteratorBase<Iterator, Transformer>::operator++() {
		++base;
		return *this;
	}

	template <class Iterator, class Transformer>
	TransformIteratorBase<Iterator, Transformer> TransformIteratorBase<Iterator, Transformer>::operator++(int) {
		return TransformIteratorBase(base++);
	}

	template <class Iterator, class Transformer>
	template <class T>
	decltype(auto) TransformIteratorBase<Iterator, Transformer>::transform(T&& arg) const {
		return Transformer{}(arg);
	}


	//------------------------------------------------------------------------------
	// Input iterator.
	//------------------------------------------------------------------------------


	template <class Iterator, class Transformer>
	TransformInputIterator<Iterator, Transformer>::TransformInputIterator(Iterator base)
		: Base(base) {}

	template <class Iterator, class Transformer>
	typename TransformInputIterator<Iterator, Transformer>::pointer TransformInputIterator<Iterator, Transformer>::operator->() const {
		return &operator*();
	}

	template <class Iterator, class Transformer>
	TransformInputIterator<Iterator, Transformer>& TransformInputIterator<Iterator, Transformer>::operator++() {
		Base::operator++();
		return *this;
	}

	template <class Iterator, class Transformer>
	TransformInputIterator<Iterator, Transformer> TransformInputIterator<Iterator, Transformer>::operator++(int) {
		return { Base::operator++().base };
	}

	template <class Iterator, class Transformer>
	bool TransformInputIterator<Iterator, Transformer>::operator==(const TransformInputIterator& rhs) {
		return base == rhs.base;
	}

	template <class Iterator, class Transformer>
	bool TransformInputIterator<Iterator, Transformer>::operator!=(const TransformInputIterator& rhs) {
		return base != rhs.base;
	}


	//------------------------------------------------------------------------------
	// Forward iterator.
	//------------------------------------------------------------------------------

	template <class Iterator, class Transformer>
	TransformForwardIterator<Iterator, Transformer>::TransformForwardIterator(Iterator base)
		: Base(base) {}

	template <class Iterator, class Transformer>
	TransformForwardIterator<Iterator, Transformer>& TransformForwardIterator<Iterator, Transformer>::operator++() {
		Base::operator++();
		return *this;
	}

	template <class Iterator, class Transformer>
	TransformForwardIterator<Iterator, Transformer> TransformForwardIterator<Iterator, Transformer>::operator++(int) {
		return { Base::operator++().base };
	}

	template <class Iterator, class Transformer>
	bool TransformForwardIterator<Iterator, Transformer>::operator==(const TransformForwardIterator& rhs) {
		return Base::operator==(rhs);
	}

	template <class Iterator, class Transformer>
	bool TransformForwardIterator<Iterator, Transformer>::operator!=(const TransformForwardIterator& rhs) {
		return Base::operator!=(rhs);
	}


	//------------------------------------------------------------------------------
	// Bidirectional iterator.
	//------------------------------------------------------------------------------

	template <class Iterator, class Transformer>
	TransformBidirectionalIterator<Iterator, Transformer>::TransformBidirectionalIterator(Iterator base)
		: Base(base) {}

	template <class Iterator, class Transformer>
	TransformBidirectionalIterator<Iterator, Transformer>& TransformBidirectionalIterator<Iterator, Transformer>::operator++() {
		Base::operator++();
		return *this;
	}

	template <class Iterator, class Transformer>
	TransformBidirectionalIterator<Iterator, Transformer> TransformBidirectionalIterator<Iterator, Transformer>::operator++(int) {
		return { Base::operator++().base };
	}

	template <class Iterator, class Transformer>
	TransformBidirectionalIterator<Iterator, Transformer>& TransformBidirectionalIterator<Iterator, Transformer>::operator--() {
		--base;
		return *this;
	}

	template <class Iterator, class Transformer>
	TransformBidirectionalIterator<Iterator, Transformer> TransformBidirectionalIterator<Iterator, Transformer>::operator--(int) {
		return { base-- };
	}

	template <class Iterator, class Transformer>
	bool TransformBidirectionalIterator<Iterator, Transformer>::operator==(const TransformBidirectionalIterator& rhs) {
		return Base::operator==(rhs);
	}

	template <class Iterator, class Transformer>
	bool TransformBidirectionalIterator<Iterator, Transformer>::operator!=(const TransformBidirectionalIterator& rhs) {
		return Base::operator!=(rhs);
	}


	//------------------------------------------------------------------------------
	// Random access iterator.
	//------------------------------------------------------------------------------


	template <class Iterator, class Transformer>
	TransformRandomIterator<Iterator, Transformer>::TransformRandomIterator(Iterator base)
		: Base(base) {}

	template <class Iterator, class Transformer>
	TransformRandomIterator<Iterator, Transformer>& TransformRandomIterator<Iterator, Transformer>::operator++() {
		Base::operator++();
		return *this;
	}

	template <class Iterator, class Transformer>
	TransformRandomIterator<Iterator, Transformer> TransformRandomIterator<Iterator, Transformer>::operator++(int) {
		return { Base::operator++().base };
	}

	template <class Iterator, class Transformer>
	TransformRandomIterator<Iterator, Transformer>& TransformRandomIterator<Iterator, Transformer>::operator--() {
		Base::operator--();
		return *this;
	}

	template <class Iterator, class Transformer>
	TransformRandomIterator<Iterator, Transformer> TransformRandomIterator<Iterator, Transformer>::operator--(int) {
		return { Base::operator--().base };
	}

	template <class Iterator, class Transformer>
	TransformRandomIterator<Iterator, Transformer>& TransformRandomIterator<Iterator, Transformer>::operator+=(difference_type n) {
		base += n;
		return *this;
	}

	template <class Iterator, class Transformer>
	TransformRandomIterator<Iterator, Transformer> TransformRandomIterator<Iterator, Transformer>::operator+(difference_type n) const {
		return { base + n };
	}

	template <class Iterator, class Transformer>
	TransformRandomIterator<Iterator, Transformer>& TransformRandomIterator<Iterator, Transformer>::operator-=(difference_type n) {
		base -= n;
		return *this;
	}

	template <class Iterator, class Transformer>
	TransformRandomIterator<Iterator, Transformer> TransformRandomIterator<Iterator, Transformer>::operator-(difference_type n) const {
		return { base - n };
	}

	template <class Iterator, class Transformer>
	typename TransformRandomIterator<Iterator, Transformer>::difference_type TransformRandomIterator<Iterator, Transformer>::operator-(const TransformRandomIterator& i) const {
		return base - i.base;
	}

	template <class Iterator, class Transformer>
	typename TransformBidirectionalIterator<Iterator, Transformer>::reference TransformRandomIterator<Iterator, Transformer>::operator[](difference_type n) const {
		return *(*this + n);
	}

	template <class Iterator, class Transformer>
	bool TransformRandomIterator<Iterator, Transformer>::operator==(const TransformRandomIterator& rhs) {
		return Base::operator==(rhs);
	}

	template <class Iterator, class Transformer>
	bool TransformRandomIterator<Iterator, Transformer>::operator!=(const TransformRandomIterator& rhs) {
		return Base::operator!=(rhs);
	}

	template <class Iterator, class Transformer>
	bool TransformRandomIterator<Iterator, Transformer>::operator<(const TransformRandomIterator& rhs) const {
		return base < rhs.base;
	}

	template <class Iterator, class Transformer>
	bool TransformRandomIterator<Iterator, Transformer>::operator>(const TransformRandomIterator& rhs) const {
		return base > rhs.base;
	}

	template <class Iterator, class Transformer>
	bool TransformRandomIterator<Iterator, Transformer>::operator<=(const TransformRandomIterator& rhs) const {
		return base <= rhs.base;
	}

	template <class Iterator, class Transformer>
	bool TransformRandomIterator<Iterator, Transformer>::operator>=(const TransformRandomIterator& rhs) const {
		return base >= rhs.base;
	}
} // namespace impl
} // namespace inl