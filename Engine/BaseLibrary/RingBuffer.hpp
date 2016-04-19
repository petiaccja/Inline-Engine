#pragma once

#include <list>

namespace exc {


template <typename T, typename ContainerT = std::list<T>>
class RingBuffer {
protected:
	using ContainerIter = typename ContainerT::iterator;

public:
	class Iterator {
	public:
		friend class RingBuffer;

		T& operator*() {
			return *m_nativeIterator;
		}
		Iterator& operator++() {
			m_offset += 1;
			m_nativeIterator = RotateFrontNative(*m_pContainer, m_nativeIterator);
			return *this;
		}
		Iterator operator++(int) {
			Iterator copy{*this};
			++(*this);
			return copy;
		}
		Iterator& operator--() {
			m_offset -= 1;
			m_nativeIterator = RotateBackNative(*m_pContainer, m_nativeIterator);
			return *this;
		}
		Iterator operator--(int) {
			Iterator copy{*this};
			--(*this);
			return copy;
		}
		bool operator!=(const Iterator& other) {
			if (m_offset != other.m_offset) {
				return true;
			}
			return false;
		}
	protected:
		ContainerT* m_pContainer;
		ContainerIter m_nativeIterator;
		int64_t m_offset;

	}; // class Iterator


public:

	// Construction and assignement

	RingBuffer() {
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

	size_t Size() const {
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


	// Access

	T& Front() {
		return *m_currBegin;
	}

	const T& Front() const {
		return *m_currBegin;
	}


	// Modify

	void PushFront(const T& element) {
		m_currBegin = m_container.insert(m_currBegin, element);
	}

	void PushFront(T&& element) {
		m_currBegin = m_container.insert(m_currBegin, std::move(element));
	}

	/// After this function, the element at front will become the element at back
	void RotateFront() {
		m_currBegin = RotateFrontNative(m_container, m_currBegin);
		m_currOffset += 1;
	}

protected:
	ContainerT m_container;
	ContainerIter m_currBegin;
	int64_t m_currOffset;

private:
	static ContainerIter RotateFrontNative(ContainerT& container, ContainerIter target) {
		if (container.size() == 0) {
			return target;
		}

		if (++target == container.end()) {
			target = container.begin();
		}
		return target;
	}

	static ContainerIter RotateBackNative(ContainerT& container, ContainerIter target) {
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
typename exc::RingBuffer<T, C>::Iterator begin(exc::RingBuffer<T, C>& buffer) {
	return buffer.Begin();
}

template<typename T, typename C>
typename exc::RingBuffer<T, C>::Iterator end(exc::RingBuffer<T, C>& buffer) {
	return buffer.End();
}


} // namespace exc

