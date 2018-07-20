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