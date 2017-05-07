#include <type_traits>


namespace exc {

/// <summary> Helper class to wrap enums into bitflags. </summary>
/// <remarks>
///	<para> Contains operators for common operations on bitflags
///		such as union (+), set difference (-) and intersection (&). </para>
/// <para> The first template parameter is a struct containing the definition
///		of a non-scoped enum, the second parameter is the enum itself. The class
///		inherits from the struct. Unfortunately, C++ does not support inheriting
///		from enum classes. </para>		
/// </remarks>
template <class HoldingStruct, class InnerEnum>
class BitFlagEnum : public HoldingStruct {
public:
	using EnumT = InnerEnum;
private:
	using UnderlyingT = typename std::underlying_type<EnumT>::type;
public:
	/// <summary> Initializes an empty flag. </summary>
	BitFlagEnum() : m_value((EnumT)0) {}

	/// <sumamry> Init the flag to the specified value. </summary>
	BitFlagEnum(EnumT value) : m_value(value) {}

	/// <sumamry> Init the flag to the specified value. </summary>
	BitFlagEnum(const BitFlagEnum& rhs) = default;

	BitFlagEnum(std::initializer_list<EnumT> values) {
		m_value = (EnumT)0;
		for (auto v : values) {
			*this += v;
		}
	}


	/// <summary> Assign the specified value. </summary>
	BitFlagEnum& operator=(const BitFlagEnum& rhs) = default;


	/// <summary> Check if the two contain EXACTLY the same flags.
	bool operator==(BitFlagEnum rhs) const {
		return m_value == rhs.m_value;
	}
	/// <summary> Check if the two has ANY difference. </summary>
	bool operator!=(BitFlagEnum rhs) const {
		return m_value != rhs.m_value;
	}

	/// <summary> Assign the union of the two to the left side. </summary>
	BitFlagEnum& operator+=(BitFlagEnum flag) {
		(UnderlyingT&)m_value |= (UnderlyingT)flag.m_value;
		return *this;
	}
	/// <summary> Create the union of the two flags. </summary>
	BitFlagEnum operator+(BitFlagEnum flag) const {
		BitFlagEnum copy(*this);
		copy += flag;
		return copy;
	}


	/// <summary> Unset all flags in the left side that are set in the right side, that is, difference of sets. </summary>
	BitFlagEnum& operator-=(BitFlagEnum flag) {
		(UnderlyingT&)m_value &= (UnderlyingT)(~flag.m_value);
		return *this;
	}
	/// <summary> Return the set difference LEFT \ RIGHT. </summary>
	BitFlagEnum operator-(BitFlagEnum flag) const {
		BitFlagEnum copy(*this);
		copy -= flag;
		return copy;
	}


	/// <summary> Set only flags that are also set in the right side, that is, intersection of sets. </summary>
	BitFlagEnum& operator&=(BitFlagEnum rhs) {
		(UnderlyingT&)m_value &= (UnderlyingT)rhs.m_value;
		return *this;
	}
	/// <summary> Return the intersection of the two sets. </summary>
	BitFlagEnum operator&(BitFlagEnum flag) const {
		BitFlagEnum copy(*this);
		copy &= flag;
		return copy;
	}

	/// <summary> Check if any flag is set. </summary>
	/// <returns> True if not an empty bitset, false if empty. </returns> 
	explicit operator bool() {
		return (UnderlyingT)m_value != 0;
	}

	/// <summary> Cast to underlying enumeration type. </summary>
	explicit operator EnumT() {
		return m_value;
	}
private:
	EnumT m_value;
};


} // namespace exc