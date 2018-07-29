#include "Node.hpp"


#include <regex>



std::string NodeBase::GetClassName(bool simplify, const std::vector<std::regex>& additional) const {
	std::string name = typeid(*this).name();

	if (simplify) {
		// Remove class, struct and enum specifiers.
		static std::regex classFilter(R"(\s*class\s*)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		name = std::regex_replace(name, classFilter, "");

		static std::regex structFilter(R"(\s*struct\s*)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		name = std::regex_replace(name, structFilter, "");

		static std::regex enumFilter(R"(\s*enum\s*)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		name = std::regex_replace(name, enumFilter, "");

		// MSVC specific things.
		static std::regex ptrFilter(R"(\s*__ptr64\s*)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		name = std::regex_replace(name, ptrFilter, "");

		// Transform common templates to readable format
		static std::regex stringFilter(R"(\s*std::basic_string.*)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		name = std::regex_replace(name, stringFilter, "std::string");

		// Remove consts.
		static std::regex constFilter(R"(\s*const\s*)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		name = std::regex_replace(name, constFilter, "");

		// Remove inl::gxeng::
		static std::regex namespaceFilter1(R"(\s*inl::gxeng::\s*)", std::regex_constants::ECMAScript | std::regex_constants::optimize);
		name = std::regex_replace(name, namespaceFilter1, "");

		// Remove requested namespaces.
		for (auto& filter : additional) {
			name = std::regex_replace(name, filter, "");
		}
	}

	return name;
}