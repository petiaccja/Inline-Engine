#pragma once

#include <filesystem>


struct LoadTestLevelActio {};


struct LoadLevelAction {
	std::filesystem::path fileName;
};


struct SaveLevelAction {
	std::filesystem::path fileName;
};


struct ClearLevelAction {};
