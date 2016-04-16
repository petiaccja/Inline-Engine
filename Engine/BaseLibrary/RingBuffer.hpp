#pragma once

#include <list>

namespace exc {


template <typename T, typename ContainerT = std::list<T>>
class RingBuffer {
public:
	class Iterator {
	public:
		friend class RingBuffer;

		T& operator*() {
			return *m_nativeIterator;
		}
		Iterator& operator++() {
			m_offsetFromNativeBegin += 1;
			RotateFrontNative(*m_pContainer, m_nativeIterator);
			return *this;
		}
		Iterator operator++(int) {
			Iterator copy{*this};
			m_offsetFromNativeBegin += 1;
			RotateFrontNative(*m_pContainer, m_nativeIterator);
			return copy;
		}
		Iterator& operator--() {
			m_offsetFromNativeBegin -= 1;
			RotateBackNative(*m_pContainer, m_nativeIterator);
			return *this;
		}
		Iterator operator--(int) {
			Iterator copy{*this};
			m_offsetFromNativeBegin -= 1;
			RotateBackNative(*m_pContainer, m_nativeIterator);
			return copy;
		}
		bool operator!=(const Iterator& other) {
			if (m_offsetFromNativeBegin != other.m_offsetFromNativeBegin) {
				return true;
			}
			return false;
		}
	protected:
		ContainerT* m_pContainer;
		typename ContainerT::iterator m_nativeIterator;
		int  m_offsetFromNativeBegin;

	}; // class Iterator


public:

	RingBuffer() {
		m_currBegin = m_container.begin();
	}

	size_t Size() const {
		return m_container.size();
	}

	Iterator Begin() {
		Iterator beginIter;
		beginIter.m_pContainer = &m_container;
		beginIter.m_nativeIterator = m_container.begin();
		beginIter.m_offsetFromNativeBegin = 0;
		return beginIter;
	}

	Iterator End() {
		Iterator endIter;
		endIter.m_pContainer = &m_container;
		endIter.m_nativeIterator = m_container.end();
		endIter.m_offsetFromNativeBegin = static_cast<int>(m_container.size());
		return endIter;
	}

	T& Front() {
		return *m_currBegin;
	}

	const T& Front() const {
		return *m_currBegin;
	}

	void PushFront(const T& element) {
		m_container.insert(m_currBegin, element);
	}

	void PushFront(T&& element) {
		m_container.insert(m_currBegin, std::move(element));
	}

	/// After this function, the element at front will become the element at back
	void RotateFront() {
		RotateFrontNative(m_container, m_currBegin);
	}

protected:
	ContainerT m_container;
	typename ContainerT::iterator m_currBegin;

private:
	static void RotateFrontNative(ContainerT& container, typename ContainerT::iterator& target) {
		if (++target == container.end()) {
			target = container.begin();
		}
	}

	static void RotateBackNative(ContainerT& container, typename ContainerT::iterator& target) {
		if (--target == container.rend()) {
			target = container.rbegin();
		}
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

