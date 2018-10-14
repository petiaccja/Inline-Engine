#include <BaseLibrary/StringUtil.hpp>

#include <Catch2/catch.hpp>

using namespace inl;

TEST_CASE("Tokenize, no trim, cstring", "[StringUtil]") {
	auto tokens = Tokenize("the day i learn to fly", " ", false);

	std::vector<std::string_view> expectedTokens = {
		"the", "day", "i", "learn", "to", "fly"
	};

	REQUIRE(tokens == expectedTokens);
}


TEST_CASE("Tokenize, no trim, mixed view", "[StringUtil]") {
	std::string_view str("i'm never coming down");
	auto tokens = Tokenize(str, " '", false);

	std::vector<std::string_view> expectedTokens = {
		"i", "m",  "never", "coming", "down"
	};

	REQUIRE(tokens == expectedTokens);
}


TEST_CASE("Tokenize, no trim, mixed string", "[StringUtil]") {
	std::string str("on perfect  wings i'll rise");
	auto tokens = Tokenize(str, " '", false);

	std::vector<std::string_view> expectedTokens = {
		"on", "perfect", "", "wings", "i", "ll", "rise"
	};

	REQUIRE(tokens == expectedTokens);
}


TEST_CASE("Tokenize, trim", "[StringUtil]") {
	auto tokens = Tokenize("through   the  \tlayers \r\nof the clouds", " \t\r\n", true);

	std::vector<std::string_view> expectedTokens = {
		"through", "the", "layers", "of", "the", "clouds"
	};

	REQUIRE(tokens == expectedTokens);
}


TEST_CASE("Trim, simple", "[StringUtil]") {
	std::string str = " \t asd  ";
	auto trimmed = Trim(str, " \t\n");
	REQUIRE(trimmed == "asd");
}


TEST_CASE("Trim, already trimmed", "[StringUtil]") {
	std::string str = "asd";
	auto trimmed = Trim(str, " \t\n");
	REQUIRE(trimmed == "asd");
}


TEST_CASE("Trim, empty", "[StringUtil]") {
	std::string str = "";
	auto trimmed = Trim(str, " \t\n");
	REQUIRE(trimmed == "");
}


TEST_CASE("Encode UCS4->UTF-8 character", "[StringEncode]") {
	// Examples from wikipedia: https://en.wikipedia.org/wiki/UTF-8
	char32_t c1 = 0x0024;
	char32_t c2 = 0x00A2;
	char32_t c3 = 0x20AC;
	char32_t c4 = 0x10348;
	std::array<char, 4> u1exp = { 0x24, 0, 0, 0 };
	std::array<char, 4> u2exp = { 0xC2, 0xA2, 0, 0 };
	std::array<char, 4> u3exp = { 0xE2, 0x82, 0xAC, 0 };
	std::array<char, 4> u4exp = { 0xF0, 0x90, 0x8D, 0x88 };

	auto u1 = inl::impl::EncodeProduce(c1, char());
	auto u2 = inl::impl::EncodeProduce(c2, char());
	auto u3 = inl::impl::EncodeProduce(c3, char());
	auto u4 = inl::impl::EncodeProduce(c4, char());

	REQUIRE(u1exp == u1);
	REQUIRE(u2exp == u2);
	REQUIRE(u3exp == u3);
	REQUIRE(u4exp == u4);
}


TEST_CASE("Encode UCS4->UTF-16 character", "[StringEncode]") {
	// Examples from wikipedia: https://en.wikipedia.org/wiki/UTF-16
	char32_t c1 = 0x20AC;
	char32_t c2 = 0x24B62;
	std::array<char16_t, 2> u1exp = { 0x20AC, 0 };
	std::array<char16_t, 2> u2exp = { 0xD852, 0xDF62 };

	auto u1 = inl::impl::EncodeProduce(c1, char16_t());
	auto u2 = inl::impl::EncodeProduce(c2, char16_t());

	REQUIRE(u1exp == u1);
	REQUIRE(u2exp == u2);
}


TEST_CASE("Decode UTF-16->UCS4 character", "[StringEncode]") {
	std::array<char16_t, 2> u1in = { 0x20AC, 0 };
	std::array<char16_t, 2> u2in = { 0xD852, 0xDF62 };

	auto [c1, end1] = impl::EncodeConsume(u1in.data(), u1in.data() + u1in.size());
	auto [c2, end2] = impl::EncodeConsume(u2in.data(), u2in.data() + u2in.size());

	REQUIRE(c1 == 0x20AC);
	REQUIRE(c2 == 0x24B62);
}


TEST_CASE("Decode UTF-8->UCS4 character", "[StringEncode]") {
	// Examples from wikipedia: https://en.wikipedia.org/wiki/UTF-8
	std::array<char, 4> u1in = { 0x24, 0, 0, 0 };
	std::array<char, 4> u2in = { 0xC2, 0xA2, 0, 0 };
	std::array<char, 4> u3in = { 0xE2, 0x82, 0xAC, 0 };
	std::array<char, 4> u4in = { 0xF0, 0x90, 0x8D, 0x88 };

	auto [c1, end1] = impl::EncodeConsume(u1in.data(), u1in.data() + u1in.size());
	auto [c2, end2] = impl::EncodeConsume(u2in.data(), u2in.data() + u2in.size());
	auto [c3, end3] = impl::EncodeConsume(u3in.data(), u3in.data() + u3in.size());
	auto [c4, end4] = impl::EncodeConsume(u4in.data(), u4in.data() + u4in.size());

	REQUIRE(c1 == 0x0024);
	REQUIRE(c2 == 0x00A2);
	REQUIRE(c3 == 0x20AC);
	REQUIRE(c4 == 0x10348);
}


TEST_CASE("Transcode string UTF-8 -> UTF-32 -> UTF-8", "[StringEncode]") {
	std::string original = u8"На берегу пустынных волн";
	std::u32string interm = EncodeString<char32_t>(original);
	std::string recoded = EncodeString<char>(interm);

	REQUIRE(original == recoded);
}


TEST_CASE("Transcode string UTF-16 -> UTF-32 -> UTF-16", "[StringEncode]") {
	std::u16string original = u"На берегу пустынных волн";
	std::u32string interm = EncodeString<char32_t>(original);
	std::u16string recoded = EncodeString<char16_t>(interm);

	REQUIRE(original == recoded);
}