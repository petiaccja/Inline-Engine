#include "System.hpp"
#include "../../Exception/Exception.hpp"


namespace inl {


// Dll
ModuleHandle System::LoadModule(const char* path) {
	ModuleHandle h = LoadLibraryA(path);
	if (h == NULL) {
		throw FileNotFoundException("Library was not found.");
	}
	return h;
}
void System::UnloadModule(ModuleHandle handle) noexcept {
	FreeLibrary(handle);
}
void* System::GetModuleSymbolAddress(ModuleHandle handle, const char* symbolName) noexcept {
	return GetProcAddress(handle, symbolName);
}

// Cursor
Vec2i System::GetCursorPosition() {
	POINT p;
	GetCursorPos(&p);
	return { p.x, p.y };
}
void System::SetCursorPosition(const Vec2i& pos) {
	SetCursorPos(pos.x, pos.y);
}
void System::SetCursorVisual(eCursorVisual visual) {
	throw NotImplementedException("bazmeeeg");
}
void System::SetCursorVisible(bool visible) {
	ShowCursor(visible);
}

// File paths
std::string System::GetExecutableDir() {
	char name[1024];
	GetModuleFileNameA(GetModuleHandleA(nullptr), name, sizeof(name));
	std::string s(name);
	auto idx = s.find_last_of('\\');
	if (idx != s.npos) {
		return s.substr(0, idx);
	}
	else {
		return s;
	}
}


} // namespace inl