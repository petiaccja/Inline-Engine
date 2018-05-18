#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <algorithm>

namespace inl {


//------------------------------------------------------------------------------
// Makes a string view out of
// - c strings
// - std::strings
// - std::string_views
//------------------------------------------------------------------------------
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



//------------------------------------------------------------------------------
// Tokenize
//------------------------------------------------------------------------------


namespace impl {


template <class CharT, class Traits>
auto NextToken(const std::basic_string_view<CharT, Traits>& str,
			   const std::basic_string_view<CharT, Traits>& delimiters)
	-> std::optional<std::basic_string_view<CharT, Traits>>
{
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
	-> std::vector<std::basic_string_view<CharT, Traits>>
{
	using StringViewT = std::basic_string_view<CharT, Traits>;

	std::vector<StringViewT> tokens;
	std::optional<StringViewT> token;
	StringViewT currentView = str;
	while (token = NextToken(currentView, delimiters)) {
		if (!token.value().empty() || !trimEmpty) {
			tokens.push_back(token.value());
		}
		currentView = currentView.substr(std::min(currentView.size(), token.value().size() + 1));
	}	
	return tokens;
}


} // namespace inl



template <class StringT, class DelimitersT>
auto NextToken(const StringT& str,
			   const DelimitersT& delimiters)
{
	return impl::NextToken(MakeStringView(str), MakeStringView(delimiters));
}


template <class StringT, class DelimitersT>
auto Tokenize(const StringT& str,
			  const DelimitersT& delimiters,
			  bool trimEmpty = false)
{
	return impl::TokenizeStringView(MakeStringView(str), MakeStringView(delimiters), trimEmpty);
}







} // namespace inl
