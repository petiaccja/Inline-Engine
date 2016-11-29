#pragma once

#include <list>
#include <cassert>

namespace exc {


template <typename T, typename ContainerT = std::list<T>>
class RingBuffer {
protected:
	using ContainerIter = typename ContainerT::iterator;
	using ContainerConstIter = typename ContainerT::const_iterator;


	// template iterator to minimize code duplication for iterator and const iterator

	template <typename IterContainerT, typename IterContainerIterT>
	class TemplateIterator :
		public std::iterator <
			std::bidirectional_iterator_tag,
			typename IterContainerIterT::value_type
		>
	{
	public:
		friend class RingBuffer;

		value_type& operator*() {
			return *m_nativeIterator;
		}
		TemplateIterator& operator++() {
			m_offset += 1;
			m_nativeIterator = RotateFrontNative(*m_pContainer, m_nativeIterator);
			return *this;
		}
		TemplateIterator operator++(int) {
			TemplateIterator copy{*this};
			++(*this);
			return copy;
		}
		TemplateIterator& operator--() {
			m_offset -= 1;
			m_nativeIterator = RotateBackNative(*m_pContainer, m_nativeIterator);
			return *this;
		}
		TemplateIterator operator--(int) {
			TemplateIterator copy{*this};
			--(*this);
			return copy;
		}
		inline bool operator==(const TemplateIterator& other) const {
			assert(m_pContainer == other.m_pContainer);
			return m_offset == other.m_offset;
		}
		inline bool operator!=(const TemplateIterator& other) const {
			assert(m_pContainer == other.m_pContainer);
			return !(*this == other);
		}
		// Adds "count" number of rounds to the iterator.
		// This means that in order to reach the resulting iterator
		// one would have to go around the whole ring count more
		// rounds compared to the iterator the function is called on.
		// Count may be negative. In this case the function
		// decrements round count.
		TemplateIterator AddRounds(int32_t count) const {
			TemplateIterator result{*this};
			result.m_offset += count * m_pContainer->size();
			return result;
		}
	protected:
		IterContainerT* m_pContainer;
		IterContainerIterT m_nativeIterator;
		int64_t m_offset;
	}; // TemplateIterator


public:

	using Iterator = TemplateIterator<ContainerT, ContainerIter>;
	using ConstIterator = TemplateIterator<const ContainerT, ContainerConstIter>;

public:

	// Construction and assignement

	RingBuffer() : m_currOffset{0} {
		m_currBegin = m_container.begin();
	}

	RingBuffer(const RingBuffer& other) :
		m_container{other.m_container},
		m_currOffset{other.m_currOffset}
	{
		m_currBegin = std::advance(m_container.begin(), m_currOffset % m_container.size());
	}

	RingBuffer& operator=(RingBuffer other) {
		std::swap(*this, other);
		return *this;
	}

	RingBuffer(RingBuffer&&) = default;
	RingBuffer& operator=(RingBuffer&&) = default;


	// Properties

	size_t Count() const {
		return m_container.size();
	}


	// Iterators

	Iterator Begin() {
		Iterator beginIter;
		beginIter.m_pContainer = &m_container;
		beginIter.m_nativeIterator = m_currBegin;
		beginIter.m_offset = m_currOffset;
		return beginIter;
	}

	Iterator End() {
		Iterator endIter;
		endIter.m_pContainer = &m_container;
		endIter.m_nativeIterator = m_currBegin; //Fun fact: m_currBegin is actually the end at the same time
		endIter.m_offset = m_currOffset + static_cast<int64_t>(m_container.size());
		return endIter;
	}

	ConstIterator Begin() const {
		ConstIterator beginIter;
		beginIter.m_pContainer = &m_container;
		beginIter.m_nativeIterator = m_currBegin;
		beginIter.m_offset = m_currOffset;
		return beginIter;
	}

	ConstIterator End() const {
		ConstIterator endIter;
		endIter.m_pContainer = &m_container;
		endIter.m_nativeIterator = m_currBegin;
		endIter.m_offset = m_currOffset + static_cast<int64_t>(m_container.size());
		return endIter;
	}


	// Access

	T& Front() {
		return *m_currBegin;
	}

	const T& Front() const {
		return *m_currBegin;
	}

	T& Back() {
		return *RotateBackNative(m_container, m_currBegin);
	}

	const T& Back() const {
		return *RotateBackNative(m_container, m_currBegin);
	}


	// Modify

	void PushFront(const T& element) {
		m_currBegin = m_container.insert(m_currBegin, element);
	}

	void PushFront(T&& element) {
		m_currBegin = m_container.insert(m_currBegin, std::move(element));
	}

	void PopFront() {
		m_currBegin = m_container.erase(m_currBegin);
	}

	/// After this function, the element at front will become the element at back
	void RotateFront() {
		m_currBegin = RotateFrontNative(m_container, m_currBegin);
		m_currOffset += 1;
	}

	void RotateBack() {
		m_currBegin = RotateBackNative(m_container, m_currBegin);
		m_currOffset -= 1;
	}

protected:
	ContainerT m_container;
	ContainerIter m_currBegin;
	int64_t m_currOffset;

private:

	template <typename Fun_ContainerT, typename Fun_IterT>
	static Fun_IterT RotateFrontNative(Fun_ContainerT& container, Fun_IterT target) {
		if (container.size() == 0) {
			return target;
		}

		if (++target == container.end()) {
			target = container.begin();
		}
		return target;
	}

	template <typename Fun_ContainerT, typename Fun_IterT>
	static Fun_IterT RotateBackNative(Fun_ContainerT& container, Fun_IterT target) {
		if (container.size() == 0) {
			return target;
		}

		if (target == container.begin()) {
			target = --container.end();
		} 
		else {
			--target;
		}
		return target;
	}
};

// begin and end for "range-based for"
template<typename T, typename C>
typename RingBuffer<T, C>::Iterator begin(RingBuffer<T, C>& buffer) {
	return buffer.Begin();
}

template<typename T, typename C>
typename RingBuffer<T, C>::Iterator end(RingBuffer<T, C>& buffer) {
	return buffer.End();
}

template<typename T, typename C>
typename RingBuffer<T, C>::ConstIterator cbegin(const RingBuffer<T, C>& buffer) {
	return buffer.Begin();
}

template<typename T, typename C>
typename RingBuffer<T, C>::ConstIterator cend(const RingBuffer<T, C>& buffer) {
	return buffer.End();
}


} // namespace exc

