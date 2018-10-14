#pragma once

#include "Exception/Exception.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace inl {


//------------------------------------------------------------------------------
// Makes a string view out of
// - c strings
// - std::strings
// - std::string_views
//------------------------------------------------------------------------------

namespace impl {

	template <class StringT>
	void MakeStringView(const StringT& str) {
		static_assert(false, "No string view for provided type.");
	}

	template <class CharT, class CharTraits, class Allocator>
	std::basic_string_view<CharT, CharTraits> MakeStringView(const std::basic_string<CharT, CharTraits, Allocator>& str) {
		return str;
	}

	template <class CharT, class CharTraits>
	std::basic_string_view<CharT, CharTraits> MakeStringView(const std::basic_string_view<CharT, CharTraits>& str) {
		return str;
	}

	template <class CharT>
	std::basic_string_view<CharT> MakeStringView(const CharT* str) {
		return str;
	}

} // namespace impl



//------------------------------------------------------------------------------
// Tokenize
//------------------------------------------------------------------------------


namespace impl {


	template <class CharT, class Traits>
	auto NextToken(const std::basic_string_view<CharT, Traits>& str,
				   const std::basic_string_view<CharT, Traits>& delimiters)
		-> std::optional<std::basic_string_view<CharT, Traits>> {
		if (str.empty()) {
			return {};
		}

		auto pos = str.find_first_of(delimiters);
		return str.substr(0, pos);
	}


	template <class CharT, class Traits>
	auto TokenizeStringView(const std::basic_string_view<CharT, Traits>& str,
							const std::basic_string_view<CharT, Traits>& delimiters,
							bool trimEmpty = true)
		-> std::vector<std::basic_string_view<CharT, Traits>> {
		using StringViewT = std::basic_string_view<CharT, Traits>;

		std::vector<StringViewT> tokens;
		std::optional<StringViewT> token;
		StringViewT currentView = str;
		while ((token = NextToken(currentView, delimiters))) {
			if (!token.value().empty() || !trimEmpty) {
				tokens.push_back(token.value());
			}
			currentView = currentView.substr(std::min(currentView.size(), token.value().size() + 1));
		}
		return tokens;
	}


} // namespace impl



/// <summary> Splits <paramref name="str"> into tokens one token at a time. </summary>
/// <param name="str"> An std::string, std::string_view or char* to tokenize (any underlying char type). </param>
/// <param name="delimiters"> List of delimiters that separate tokens from each-other.
///		An std::string, std::string_view or char*, same underlying type as <paramref name="str"/>. </param>
/// <returns> A string_view to the first token in <paramref name="str"/>. </returns>
template <class StringT, class DelimitersT>
auto NextToken(const StringT& str,
			   const DelimitersT& delimiters) {
	return impl::NextToken(impl::MakeStringView(str), impl::MakeStringView(delimiters));
}


/// <summary> Splits a string into tokens that are separated by delimiters. </summary>
/// <param name="str"> An std::string, std::string_view or char* to tokenize (any underlying char type). </param>
/// <param name="delimiters"> List of delimiters that separate tokens from each-other.
///		An std::string, std::string_view or char*, same underlying type as <paramref name="str"/>. </param>
/// <returns> A vector of string_views which represent the individual tokens. </returns>
template <class StringT, class DelimitersT>
auto Tokenize(const StringT& str,
			  const DelimitersT& delimiters,
			  bool trimEmpty = false) {
	return impl::TokenizeStringView(impl::MakeStringView(str), impl::MakeStringView(delimiters), trimEmpty);
}



//------------------------------------------------------------------------------
// Trim characters from begin and end.
//------------------------------------------------------------------------------


namespace impl {


	template <class CharT, class Traits>
	auto Trim(const std::basic_string_view<CharT, Traits>& str,
			  const std::basic_string_view<CharT, Traits>& delimiters)
		-> std::basic_string_view<CharT, Traits> {
		auto beginIt = str.begin();
		while (beginIt != str.end() && delimiters.find(*beginIt) != std::string::npos) {
			++beginIt;
		}

		auto endIt = str.end();
		while (endIt != beginIt) {
			--endIt;
			if (delimiters.find(*endIt) == std::string::npos) {
				++endIt;
				break;
			}
		}

		return str.substr(beginIt - str.begin(), endIt - beginIt);
	}


} // namespace impl

/// <summary> Removes unwanted characters from the beginning and end of string. </summary>
/// <param name="str"> Type is std::string, std::string_view or char*, or with char16_t/char32_t etc. </param>
/// <param name="unwanted"> List of characters you want to cut off.
///		Std::string, std::string_view or char*, but same underlying char type as <paramref name="str"/>. </param>
/// <returns> String_view to the trimmed string. </returns>
template <class StringT, class DelimitersT>
auto Trim(const StringT& str,
		  const DelimitersT& unwanted) {
	return impl::Trim(impl::MakeStringView(str), impl::MakeStringView(unwanted));
}



//------------------------------------------------------------------------------
// Encoding and conversion.
//------------------------------------------------------------------------------


namespace impl {
	inline bool HasPattern(uint8_t pattern, uint8_t mask, uint8_t target) {
		return pattern == (target & mask);
	}


	inline std::pair<char32_t, const char*> EncodeConsume(const char* begin, const char* end) {
		if (begin == end) {
			return { 0, end };
		}
		uint8_t codeUnit = (uint8_t)*begin;

		// Determine number of code units based on first code unit.
		int numCodeUnits =
			HasPattern(0b0000'0000, 0b1000'0000, codeUnit) * 1
			+ HasPattern(0b1100'0000, 0b1110'0000, codeUnit) * 2
			+ HasPattern(0b1110'0000, 0b1111'0000, codeUnit) * 3
			+ HasPattern(0b1111'0000, 0b1111'1000, codeUnit) * 4;
		if (numCodeUnits == 0) {
			throw InvalidArgumentException("Invalid UTF-8 encoding, invalid code unit.");
		}
		if (end - begin < numCodeUnits) {
			throw InvalidArgumentException("Invalid UTF-8 encoding, not enough code units.");
		}

		// Decode first code unit.
		int numValidBits = 7 - numCodeUnits + (numCodeUnits == 1);
		uint32_t codePoint = codeUnit & ~(0xFF << numValidBits);
		for (; --numCodeUnits > 0;) {
			++begin;
			codePoint <<= 6;
			codePoint += *begin & 0b0011'1111;
		}
		return { codePoint, ++begin };
	}


	inline std::pair<char32_t, const char16_t*> EncodeConsume(const char16_t* begin, const char16_t* end) {
		if (begin == end) {
			return { 0, end };
		}
		// Single 16-bit code unit.
		if (*begin <= 0xD7FF || 0xE000 <= *begin) {
			return { *begin, begin + 1 };
		}
		// Two 16-bit code units.
		char32_t highSurr = char32_t(*begin - 0xD800) * 0x400;
		++begin;
		if (begin == end) {
			throw InvalidArgumentException("Invalid UTF16 encoding, not enough code units.");
		}
		char32_t lowSurr = *begin - 0xDC00;
		return { highSurr + lowSurr + 0x10000, begin + 1 };
	}


	inline std::pair<char32_t, const char32_t*> EncodeConsume(const char32_t* begin, const char32_t* end) {
		return begin != end ? std::pair<char32_t, const char32_t*>{ *begin, begin + 1 } : std::pair<char32_t, const char32_t*>{ 0, end };
	}


	inline std::array<char, 4> EncodeProduce(char32_t input, char) {
		// Yes, these are in octal.
		if (input <= 0177) {
			return { (char)input, 0, 0, 0 };
		}
		else if (input <= 03777) {
			return { char(0300 + (input >> 6)), char(0200 + (input & 077)), 0, 0 };
		}
		else if (input <= 0177777) {
			return { char(0340 + (input >> 12)), char(0200 + ((input >> 6) & 077)), char(0200 + (input & 077)), 0 };
		}
		else if (input <= 04177777) {
			return { char(0360 + (input >> 18)), char(0200 + ((input >> 12) & 077)), char(0200 + ((input >> 6) & 077)), char(0200 + (input & 077)) };
		}
		else {
			throw InvalidArgumentException("Invalid UCS4/UTF-32 code points.");
		}
	}


	inline std::array<char16_t, 2> EncodeProduce(char32_t input, char16_t) {
		// Single 16-bit code unit.
		if ((input < 0xD7FF || 0xE000 < input) && input < 0xFFFF) {
			return { (char16_t)input, u'\0' };
		}
		// Surrogate pair.
		input -= 0x10000;
		if (input > 0xFFFFF) {
			throw InvalidArgumentException("Invalid UCS4/UTF-32 code point.");
		}
		char16_t highSurr = char16_t((input >> 10) + 0xD800);
		char16_t lowSurr = char16_t((input & 0b11'1111'1111) + 0xDC00);
		return { highSurr, lowSurr };
	}


	inline std::array<char32_t, 1> EncodeProduce(char32_t input, char32_t) {
		return { input };
	}

} // namespace impl

/// <summary> Reencodes <paramref name="input"/> string. </summary>
template <class OutChar, class OutTraits = std::char_traits<OutChar>, class OutAllocator = std::allocator<OutChar>, class InStringT = void>
std::basic_string<OutChar, OutTraits, OutAllocator> EncodeString(const InStringT& input) {
	static_assert(!std::is_same_v<OutChar, wchar_t>, "Wchar_t sucks and you suck, too.");

	std::basic_string<OutChar, OutTraits, OutAllocator> output;
	auto inputView = impl::MakeStringView(input);

	auto begin = inputView.data();
	auto end = inputView.data() + inputView.size();

	while (begin != end) {
		auto [codePoint, next] = impl::EncodeConsume(begin, end);
		auto reencode = impl::EncodeProduce(codePoint, OutChar());

		auto first = reencode.begin();
		while (first != reencode.end() && *first != 0) {
			output.push_back(*first);
			++first;
		}

		begin = next;
	}

	return output;
}



} // namespace inl
