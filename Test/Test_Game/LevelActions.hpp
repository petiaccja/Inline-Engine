#pragma once

#include <filesystem>


struct LoadLevelAction {
	std::filesystem::path fileName;
};

struct SaveLevelAction {
	std::filesystem::path fileName;
};
