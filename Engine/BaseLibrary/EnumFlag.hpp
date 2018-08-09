#pragma once

#include <type_traits>

namespace inl {


namespace impl {

template <class T>
struct IsEnumFlagSuitable {
private:
	template <class U = typename T::EnumT>
	static constexpr bool has(int) { return true; }

	template <class U>
	static constexpr bool has(...) { return false; }
public:
	static constexpr bool value = has<int>(0) && std::is_enum_v<typename T::EnumT>;
};

}


template <typename EnumFlagData>
struct EnumFlag_Helper : public EnumFlagData {
	static_assert(impl::IsEnumFlagSuitable<EnumFlagData>::value, "Enum flag data must be a struct containing a plain enum called EnumT");
public:
	using EnumT = typename EnumFlagData::EnumT;
	using UnderlyingT = typename std::underlying_type<EnumT>::type;

	/// <summary> Initializes an empty flag. </summary>
	EnumFlag_Helper() : m_value((EnumT)0) {}

	/// <sumamry> Init the flag to the specified value. </summary>
	EnumFlag_Helper(EnumT value) : m_value(value) {}

	/// <sumamry> Init the flag to the specified value. </summary>
	EnumFlag_Helper(const EnumFlag_Helper& rhs) = default;


	EnumFlag_Helper(std::initializer_list<EnumT> values) {
		m_value = (EnumT)0;
		for (auto v : values) {
			*this += v;
		}
	}

	/// <summary> Assign the specified value. </summary>
	EnumFlag_Helper& operator=(const EnumFlag_Helper& rhs) = default;


	/// <summary> Check if the two contain EXACTLY the same flags.
	bool operator==(EnumFlag_Helper rhs) const {
		return m_value == rhs.m_value;
	}
	/// <summary> Check if the two has ANY difference. </summary>
	bool operator!=(EnumFlag_Helper rhs) const {
		return m_value != rhs.m_value;
	}

	/// <summary> Assign the union of the two to the left side. </summary>
	EnumFlag_Helper& operator+=(EnumFlag_Helper flag) {
		(UnderlyingT&)m_value |= (UnderlyingT)flag.m_value;
		return *this;
	}
	/// <summary> Create the union of the two flags. </summary>
	EnumFlag_Helper operator+(EnumFlag_Helper flag) const {
		EnumFlag_Helper copy(*this);
		copy += flag;
		return copy;
	}


	/// <summary> Unset all flags in the left side that are set in the right side, that is, difference of sets. </summary>
	EnumFlag_Helper& operator-=(EnumFlag_Helper flag) {
		(UnderlyingT&)m_value &= (UnderlyingT)(~flag.m_value);
		return *this;
	}
	/// <summary> Return the set difference LEFT \ RIGHT. </summary>
	EnumFlag_Helper operator-(EnumFlag_Helper flag) const {
		EnumFlag_Helper copy(*this);
		copy -= flag;
		return copy;
	}


	/// <summary> Set only flags that are also set in the right side, that is, intersection of sets. </summary>
	EnumFlag_Helper& operator&=(EnumFlag_Helper rhs) {
		(UnderlyingT&)m_value &= (UnderlyingT)rhs.m_value;
		return *this;
	}
	/// <summary> Return the intersection of the two sets. </summary>
	EnumFlag_Helper operator&(EnumFlag_Helper flag) const {
		EnumFlag_Helper copy(*this);
		copy &= flag;
		return copy;
	}

	/// <summary> Check if any flag is set. </summary>
	/// <returns> True if not an empty bitset, false if empty. </returns> 
	explicit operator bool() const {
		return (UnderlyingT)m_value != 0;
	}

	/// <summary> Cast to underlying enumeration type. </summary>
	explicit operator EnumT() {
		return m_value;
	}


	/// <summary> Return true if no flag is set. </summary>
	bool Empty() const { return !operator bool(); }

	/// <summary> Returns true if all values of <paramref name="rhs"/> are contained in this. </summary>
	bool Contains(EnumFlag_Helper rhs) {
		return (rhs - *this).Empty();
	}
private:
	EnumT m_value;
};


}