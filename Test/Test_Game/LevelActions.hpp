#pragma once

#include <filesystem>


struct LoadTestLevelAction {};


struct LoadLevelAction {
	std::filesystem::path fileName;
};


struct SaveLevelAction {
	std::filesystem::path fileName;
};


struct ClearLevelAction {};
