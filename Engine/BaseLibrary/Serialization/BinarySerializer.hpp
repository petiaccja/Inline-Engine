#pragma once

#include <deque>
#include <cstdint>
#include <limits>
#include <iterator>
#include <type_traits>


namespace inl {


/// <summary>
/// Convert C++ types to portable byte stream.
/// </summary>
/// <remarks>
/// Conversion is provided for primitive types, that is,
/// integers, floating point and enumerations. To extend functionality for
/// complex types, overload the &lt;&lt; and &gt;&gt; operators.
/// </remarks>
class BinarySerializer {
private:
	/// <summary>
	/// Base iterator class. Satisfies RandomAccessIterator.
	/// Parametrize template with 'uint8_t' and 'const uint8_t' to get mutable and const iterators.
	/// </summary>
	template <class T>
	class iterator_base : public std::iterator<std::random_access_iterator_tag, T> {
	public:
		friend class BinarySerializer;

		iterator_base() {
			index = -1;
			parent = nullptr;
		}

		T& operator*() {
			return parent->operator[](index);
		}

		T* operator->() {
			return &parent->operator[](index);
		}

		bool operator==(const iterator_base& rhs) const {
			return (index == rhs.index) ||
				((rhs.index < 0) && (index < 0)) ||
				((rhs.index >= (ptrdiff_t)parent->Size()) && (index >= (ptrdiff_t)parent->Size()));
		}
		bool operator!=(const iterator_base& rhs) const {
			return !(*this == rhs);
		}

		iterator_base& operator++() {
			++index;
			return *this;
		}
		iterator_base operator++(int) {
			auto copy = *this;
			++index;
			return copy;
		}

		// bidirectional
		iterator_base& operator--() {
			--index;
			return *this;
		}
		iterator_base operator--(int) {
			auto copy = *this;
			--index;
			return copy;
		}

		// random access
		iterator_base& operator+=(difference_type n) {
			index += n;
			return *this;
		}
		iterator_base& operator-=(difference_type n) {
			index -= n;
			return *this;
		}
		iterator_base operator+(difference_type n) const {
			iterator_base it(*this);
			return it += n;
		}
		iterator_base operator-(difference_type n) const {
			iterator_base it(*this);
			return it -= n;
		}
		difference_type operator-(const iterator_base& rhs) const {
			return rhs.index - index;
		}

		bool operator<(const iterator_base& rhs) const {
			return index < rhs.index;
		}
		bool operator>(const iterator_base& rhs) const {
			return index > rhs.index;
		}
		bool operator<=(const iterator_base& rhs) const {
			return index <= rhs.index;
		}
		bool operator>=(const iterator_base& rhs) const {
			return index >= rhs.index;
		}

		template <class = typename std::enable_if<!std::is_const<T>::value>::type>
		operator iterator_base<const T>() {
			iterator_base<const T> it;
			it.parent = parent;
			it.index = index;
			return it;
		}
	protected:
		/// <summary> The container that created this iterator. </summary>
		/// <remarks> Note that constness is selected to match the iterator's constness. </remarks>
		typename std::conditional<std::is_const<T>::value, const BinarySerializer, BinarySerializer>::type
			*parent;
		/// <summary> The index of the element pointed to by the iterator. </summary>
		intptr_t index;
	};

	template <class T>
	friend iterator_base<T> operator+(typename iterator_base<T>::difference_type n, const iterator_base<T>& it);

public:
	/// <summary> Iterate over bytes of the stream. </summary>
	using const_iterator = iterator_base<const uint8_t>;
	/// <summary> Iterate over bytes of the stream. </summary>
	using iterator = iterator_base<uint8_t>;

public:
	// raw input

	/// <summary> Insert a byte to the stream at given position. </summary>
	/// <param name="where"> Byte is inserted right before this element. </param>
	/// <param name="byte"> The value to insert. </param>
	/// <remarks> A non-dereferencable iterator results in insertion at the end. </remarks>
	void Insert(const_iterator where, uint8_t byte);

	/// <summary> Insert multiple bytes to the stream at given position. </summary>
	/// <param name="where"> Bytes are inserted right before this element. </param>
	/// <param name="data"> A pointer to the bytes to insert. </param>
	/// <param name="size"> The number of bytes pointed by data. </param>
	/// <remarks> A non-dereferencable iterator results in insertion at the end. </remarks>
	void Insert(const_iterator where, uint8_t* data, size_t size);

	/// <summary> Insert a range of bytes into the stream at given position. </summary>
	/// <param name="where"> Bytes are inserted right before this element. </param>
	/// <param name="first"> Iterator to the first element in the range. </param>
	/// <param name="last"> Iterator past the last element in the range. This element is not inserted. </param>
	/// <remarks> A non-dereferencable iterator results in insertion at the end. </remarks>
	template <class Iter>
	void Insert(const_iterator where, Iter first, Iter last);


	/// <summary> Append a single byte to the front of the stream. </summary>
	void PushFront(uint8_t);

	/// <summary> Append an array of bytes to the front of the stream. </summary>
	/// <param name="data"> A pointer to the bytes to insert. </param>
	/// <param name="size"> Number of bytes pointed by data. </param>
	void PushFront(uint8_t* data, size_t size);

	/// <summary> Append a range of bytes to the front of the stream. </summary>
	/// <param name="first"> Iterator to the first element in the range. </param> 
	/// <param name="last"> Iterator past the last element in the range. This element is not inserted. </param>
	template <class Iter>
	void PushFront(Iter first, Iter last);


	/// <summary> Append a single byte to the end of the stream. </summary>
	void PushBack(uint8_t);

	/// <summary> Append an array of bytes to the end of the stream. </summary>
	/// <param name="data"> A pointer to the bytes to insert. </param>
	/// <param name="size"> Number of bytes pointed by data. </param>
	void PushBack(uint8_t* data, size_t size);

	/// <summary> Append a range of bytes to the end of the stream. </summary>
	/// <param name="first"> Iterator to the first element in the range. </param> 
	/// <param name="last"> Iterator past the last element in the range. This element is not inserted. </param>
	template <class Iter>
	void PushBack(Iter first, Iter last);


	/// <summary> Remove a byte from the stream's front. </summary>
	/// <remarks> Calling on an empty container results in undefined behaviour. </remarks>
	/// <returns> The value of the removed byte. </returns>
	uint8_t PopFront();

	/// <summary> Remove a byte from the stream's end. </summary>
	/// <remarks> Calling on an empty container results in undefined behaviour. </remarks>
	/// <returns> The value of the removed byte. </returns>
	uint8_t PopBack();


	/// <summary> Erase a continuous part of the byte stream. </summary>
	/// <param name="where"> The first element to be erased. </param>
	/// <param name="size"> Number of elements to erase. </param>
	/// <remarks> Erasing an inexistent range is undefined behaviour. </remarks>
	void Erase(const_iterator where, size_t size = 1);

	/// <summary> Erase a range from the stream. </summary>
	/// <param name="first"> The first element to erase. </param>
	/// <param name="last"> Iterator past the last element to erase.
	///		This item will not be erased, but the one right before will. </param>
	void Erase(const_iterator first, const_iterator last);



	// misc

	/// <summary> Get the number of bytes currently in the stream. </summary>
	size_t Size() const { return buffer.size(); }

	/// <summary> Empty the stream. </summary>
	void Clear() { buffer.clear(); }

	/// <summary> Check if the stream is empty. </summary>
	bool Empty() const { return buffer.empty(); }


	// element access

	/// <summary> Modify element of the stream at specified index. </summary>
	/// <remarks> Out-of-range indices cause undefined behaviour. </remarks>
	uint8_t& operator[](size_t index);

	/// <summary> Read element of the stream at specified index. </summary>
	/// <remarks> Out-of-range indices cause undefined behaviour. </remarks>
	const uint8_t& operator[](size_t index) const;

	/// <summary> Get iterator to the first byte of the stream. </summary>
	iterator begin();
	/// <summary> Get iterator to the element after the last byte of the stream. </summary>
	iterator end();

	/// <summary> Get const iterator to the first byte of the stream. </summary>
	const_iterator begin() const;
	/// <summary> Get const iterator to the element after the last byte of the stream. </summary>
	const_iterator end() const;

	/// <summary> Get const iterator to the first byte of the stream. </summary>
	const_iterator cbegin() const;
	/// <summary> Get const iterator to the element after the last byte of the stream. </summary>
	const_iterator cend() const;

public:
	static constexpr intptr_t EndPosition() { return std::numeric_limits<ptrdiff_t>::max(); }
	static constexpr intptr_t BeginPosition() { return 0; }

private:
	/// <summary> Contains the byte stream as a sequence on uint8_t's. </summary>
	std::deque<uint8_t> buffer;
};


template <class Iter>
void BinarySerializer::Insert(const_iterator where, Iter first, Iter last) {
	auto it = where == end() ? buffer.end() : buffer.begin() + where.index;
	buffer.insert(it, first, last);
}

template <class Iter>
void BinarySerializer::PushFront(Iter first, Iter last) {
	if (first == last) {
		return;
	}
	while (first != --last) {
		buffer.push_front(*last);
	}
}

template <class Iter>
void BinarySerializer::PushBack(Iter first, Iter last) {
	while (first != last) {
		buffer.push_back(*first);
		++first;
	}
}


template <class T>
inline BinarySerializer::iterator_base<T> operator+(
	typename BinarySerializer::iterator_base<T>::difference_type n,
	const BinarySerializer::iterator_base<T>& it)
{
	return it + n;
}


//------------------------------------------------------------------------------
// insert operators
//------------------------------------------------------------------------------

template <typename T, typename U>
struct decay_equiv :
	std::is_same<typename std::decay<T>::type, U>::type
{};

// overload for bool

/// <summary>
///	Serialize a boolean value and append to the end of the stream.
/// Size is 8 bits, LSB set to 1 for true, 0 for false, other bits are 0.
/// </summary>
inline BinarySerializer& operator << (BinarySerializer& s, bool v) {
	s.PushBack(v ? 1 : 0);
	return s;
}

// signed integer type
template <class T>
void InsertSerialized(BinarySerializer& s, const BinarySerializer::const_iterator& where,
					  T v,
					  typename std::enable_if<
					  std::is_integral<T>::value &&
					  std::is_signed<T>::value,
					  T>::type = T())
{
	uint8_t buffer[sizeof(T)];
	bool isNegative = v < 0;
	T absolute = std::abs(v);
	for (int i = 0; i < sizeof(T); i++) {
		buffer[i] = uint8_t(absolute >> (8 * (sizeof(T) - i - 1)));
	}
	if (isNegative) {
		buffer[0] |= 0b1000'0000;
	}
	s.Insert(where, buffer, sizeof(buffer));
}

// unsigned integer type
template <class T>
void InsertSerialized(BinarySerializer& s, const BinarySerializer::const_iterator& where,
					  T v,
					  typename std::enable_if<
					  std::is_integral<T>::value &&
					  !std::is_signed<T>::value,
					  T>::type = T())
{
	uint8_t buffer[sizeof(T)];
	for (int i = 0; i < sizeof(T); i++) {
		buffer[i] = uint8_t(v >> (8 * (sizeof(T) - i - 1)));
	}
	s.Insert(where, buffer, sizeof(buffer));
}

// enumeration type
template <class T>
void InsertSerialized(BinarySerializer& s, const BinarySerializer::const_iterator& where,
					  T v,
					  typename std::enable_if<
					  std::is_enum<T>::value,
					  T>::type = T())
{
	using Underlying = typename std::underlying_type<T>::type;
	InsertSerialized(s, where, (Underlying)v);
}

// overload for integral and enum types
/// <summary>
/// Serialize integer and enum value and append to the end of the stream.
/// <para> Signed integers are stored in absolute + sign format. MSB is sign bit,
///		lower bits contain the binary absolute value. </para>
/// <para> Unsigned integers are simply stored as binary. </para>
/// <para> Enum types are converted to the underlying type, then stored as integer. </para>
/// <para> Byte order is big-endian in all cases. </para>
/// </summary>
template <class T, class = typename std::enable_if<!decay_equiv<T, BinarySerializer>::value && (std::is_integral<T>::value || std::is_enum<T>::value)>::type>
BinarySerializer& operator << (BinarySerializer& s, T v)
{
	InsertSerialized(s, s.end(), v);
	return s;
};

// overload for floating point types
/// <summary>
/// Serialize 32-bit float and append to the end of the stream.
/// Format is IEEE-754 for 32 bit binary floats, big-endian.
/// </summary>
BinarySerializer& operator << (BinarySerializer& s, float v);

/// <summary>
/// Serialize 64-bit float and append to the end of the stream.
/// Format is IEEE-754 for 64 bit binary floats, big-endian.
/// </summary>
BinarySerializer& operator << (BinarySerializer& s, double v);

/// <summary>
/// Intended for 128 bit floats, not implemented yet.
/// </summary>
BinarySerializer& operator << (BinarySerializer& s, long double v) = delete;


/// <summary> Serializes and appends data to the front of the stream.
///		See appropriate right insertion operators. </summary>
template <class T, class = typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type>
BinarySerializer& operator >> (const T& v, BinarySerializer& s) {
	InsertSerialized(s, s.begin(), v);
	return s;
}

BinarySerializer& operator >> (float v, BinarySerializer& s);
BinarySerializer& operator >> (double v, BinarySerializer& s);
BinarySerializer& operator >> (long double v, BinarySerializer& s) = delete;

//------------------------------------------------------------------------------
// extract operators
//------------------------------------------------------------------------------

// overload for bool

/// <summary> Extracts a boolean from the stream. <summary>
inline BinarySerializer& operator >> (BinarySerializer& s, bool& v) {
	v = s.PopBack() > 0;
	return s;
}


// signed integer type
template <class T>
void ExtractSerialized(BinarySerializer& s, BinarySerializer::const_iterator where,
					   T& v,
					   typename std::enable_if<
					   std::is_integral<T>::value &&
					   std::is_signed<T>::value,
					   T>::type = T())
{
	T value = 0;
	uint8_t first = *where;
	auto it = where;
	bool isNegative = first < 0;
	first &= 0b0111'1111;
	value += T(first) << ((sizeof(value) - 1) * 8);

	++it;
	for (int i = 1; i < sizeof(T); ++i, ++it) {
		value += T(*it) << ((sizeof(value) - 1 - i) * 8);
	}

	if (isNegative) {
		value = -value;
	}

	s.Erase(where, sizeof(T));
	v = value;
}


// unsigned integer type
template <class T>
void ExtractSerialized(BinarySerializer& s, BinarySerializer::const_iterator where,
					   T& v,
					   typename std::enable_if<
					   std::is_integral<T>::value &&
					   !std::is_signed<T>::value,
					   T>::type = T())
{
	T value = 0;
	auto it = where;
	for (int i = 0; i < sizeof(T); ++i, ++it) {
		value += (T(*it) << ((sizeof(value) - 1 - i) * 8));
	}

	s.Erase(where, sizeof(T));
	v = value;
}

// enumeration type
template <class T>
void ExtractSerialized(BinarySerializer& s, BinarySerializer::const_iterator where,
					   T& v,
					   typename std::enable_if<
					   std::is_enum<T>::value,
					   T>::type = T())
{
	using Underlying = typename std::underlying_type<T>::type;
	Underlying v_;
	ExtractSerialized(s, where, v_);
	v = (T)v_;
}


// overload for integral and enum types
/// <summary> Extract integer an enum values from the stream. <sumamry>
template <class T, class = typename std::enable_if<!decay_equiv<T, BinarySerializer>::value && (std::is_integral<T>::value || std::is_enum<T>::value)>::type>
BinarySerializer& operator >> (BinarySerializer& s, T& v)
{
	ExtractSerialized(s, s.begin(), v);
	return s;
};


// overload for floating point types
/// <summary> Extract 32 bit IEEE-754 binary float from stream. </summary>
BinarySerializer& operator >> (BinarySerializer& s, float& v);

/// <summary> Extract 64 bit IEEE-754 binary float from stream. </summary>
BinarySerializer& operator >> (BinarySerializer& s, double& v);

/// <summary> Intended to extract 128 bit IEEE-754 binary float from stream. Not implemented yet. </summary>
BinarySerializer& operator >> (BinarySerializer& s, long double& v) = delete;


/// <summary> Extracts data from the front of the stream.
///		See appropriate right extraction operators. </summary>
template <class T, class = typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type>
BinarySerializer& operator << (T& v, BinarySerializer& s) {
	ExtractSerialized(s, s.begin(), v);
	return s;
}

BinarySerializer& operator << (float& v, BinarySerializer& s);
BinarySerializer& operator << (double& v, BinarySerializer& s);
BinarySerializer& operator << (long double& v, BinarySerializer& s) = delete;



// DELETE THESE AFTER TESTING
uint32_t FloatToIEEE754(float v);
float IEEE754ToFloat(uint32_t b);
uint64_t DoubleToIEEE754(double v);
double IEEE754ToDouble(uint64_t b);


} // !namespace inl!
