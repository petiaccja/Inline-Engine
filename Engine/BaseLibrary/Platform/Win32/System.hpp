#pragma once


#include "Input.hpp"
#include <vector>
#include <InlineMath.hpp>


namespace inl {


using ModuleHandle = HMODULE;


enum class eCursorVisual {
	ARROW,
	IBEAM,
	WAIT,
	CROSS,
	UPARROW,
	SIZE,
	ICON,
	SIZENWSE,
	SIZENESW,
	SIZEWE,
	SIZENS,
	SIZEALL,
	NO,
	HAND,
	APPSTARTING,
	HELP,
};


class System {
public:
	// Dll
	static ModuleHandle LoadModule(const char* path);
	static void UnloadModule(ModuleHandle handle) noexcept;
	static void* GetModuleSymbolAddress(ModuleHandle handle, const char* symbolName) noexcept;

	// Cursor
	static Vec2i GetCursorPosition();
	static void SetCursorPosition(const Vec2i& pos);
	static void SetCursorVisual(eCursorVisual visual);
	static void SetCursorVisible(bool visible);

	// File paths
	static std::string GetExecutableDir();
};



} // namespace inl