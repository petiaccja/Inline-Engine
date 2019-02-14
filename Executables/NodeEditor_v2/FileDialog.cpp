#include "FileDialog.hpp"

#ifndef _WIN32
static_assert(false, "oops, file dialogs are only implemented for windows. Pls make custom dialogs with the game engine's gui system.");
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <commdlg.h>
#include <vector>


namespace inl::tool {


std::optional<std::string> ShowFileOpenDialog() {
	OPENFILENAME openFileName;
	std::vector<char> path;
	path.resize(4096);
	SecureZeroMemory(&openFileName, sizeof(openFileName));
	openFileName.lStructSize = sizeof(openFileName);
	openFileName.lpstrFilter = "JSON files (.json)\0*.json\0\0";
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFile = path.data();
	openFileName.nMaxFile = (DWORD)(path.size() - 1);
	openFileName.lpstrTitle = "Select JSON pipeline file";

	BOOL ret = GetOpenFileNameA(&openFileName);
	if (ret == 0) {
		return {};
	}
	return path.data();
}
std::optional<std::string> ShowFileSaveDialog() {
	OPENFILENAME openFileName;
	std::vector<char> path;
	path.resize(4096);
	SecureZeroMemory(&openFileName, sizeof(openFileName));
	openFileName.lStructSize = sizeof(openFileName);
	openFileName.lpstrFilter = "JSON files (.json)\0*.json\0\0";
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFile = path.data();
	openFileName.nMaxFile = (DWORD)(path.size() - 1);
	openFileName.lpstrTitle = "Select JSON pipeline file";

	BOOL ret = GetSaveFileNameA(&openFileName);
	if (ret == 0) {
		return {};
	}
	return path.data();
}


} // namespace inl::tool
