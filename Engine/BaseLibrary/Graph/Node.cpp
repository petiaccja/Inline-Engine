#include "Node.hpp"


#include <regex>



std::string NodeBase::GetClassName(bool simplify, const std::vector<std::string>& stripNamespaces) const {
	std::string name = typeid(*this).name();

	if (simplify) {
		// Remove class, struct and enum specifiers.
		std::regex classFilter(R"(\s*class\s*)");
		name = std::regex_replace(name, classFilter, "");

		std::regex structFilter(R"(\s*struct\s*)");
		name = std::regex_replace(name, structFilter, "");

		std::regex enumFilter(R"(\s*enum\s*)");
		name = std::regex_replace(name, enumFilter, "");

		// MSVC specific things.
		std::regex ptrFilter(R"(\s*__ptr64\s*)");
		name = std::regex_replace(name, ptrFilter, "");

		// Transform common templates to readable format
		std::regex stringFilter(R"(\s*std::basic_string.*)");
		name = std::regex_replace(name, stringFilter, "std::string");

		// Remove consts.
		std::regex constFilter(R"(\s*const\s*)");
		name = std::regex_replace(name, constFilter, "");

		// Remove requested namespaces.
		for (auto& ns : stripNamespaces) {
			std::regex namespaceFilter1(R"(\s*)" + ns + R"(\s*)");
			name = std::regex_replace(name, namespaceFilter1, "");
		}
	}

	return name;
}