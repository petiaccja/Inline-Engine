
#include "../Sys.hpp"



//#define NOMINMAX
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinUser.h>

#include <xlocbuf>
#include <codecvt>

#include <iostream>
#include <tchar.h>

namespace inl {

static HCURSOR cursorHandle = nullptr;

DLLHandle Sys::LoadDLL(const wchar_t* path)
{
	auto value = LoadLibraryW(path);
	return value;
}

bool Sys::UnLoadDLL(DLLHandle dllHandle)
{
	return FreeLibrary((HMODULE)dllHandle) ? true : false;
}

void Sys::ShowMsgBox(const std::wstring& msg)
{
	MessageBoxW(0, msg.c_str(), L"", MB_OK);

}

void Sys::SetCursorPos(const Vec2i& pos)
{
	::SetCursorPos(pos.x, pos.y);
}

void Sys::SetCursorVisible(bool b)
{
	ShowCursor(b ? SW_SHOW : SW_HIDE);
}

void* Sys::GetDLLProcAddress(DLLHandle dllHandle, const std::string& procName)
{
	return (void*)GetProcAddress((HMODULE)dllHandle, procName.c_str());
}

// TODO REMOVE
std::wstring Sys::GetExeDirW()
{
	std::string exeDir = GetExeDir();
	return std::wstring(exeDir.begin(), exeDir.end());
}

std::string	Sys::GetExeDir()
{
	// TODO make define or constant somewhere (128)
	char path[256];
	memset(path, 0, sizeof(path));
	GetModuleFileNameA(0, path, 256);

	size_t i = 255;
	while (i > 0 && path[i] != '/' && path[i] != '\\')
		i--;

	path[i + 1] = 0;

	size_t idx = 0;
	while (path[idx] != '\0') 
	{
		if (path[idx] == '\\')
			path[idx] = '/';

		idx++;
	}

	return path;
}

Vec2 Sys::GetCursorPos()
{
	POINT p; ::GetCursorPos(&p);
	return Vec2(p.x, p.y);
}

Vec2u Sys::GetScreenSize()
{
	return Vec2u((unsigned)GetSystemMetrics(SM_CXSCREEN), (unsigned)GetSystemMetrics(SM_CYSCREEN));
}

void Sys::SetCursorVisual(eCursorVisual visual, WindowHandle hwnd /*= nullptr*/)
{
	switch (visual)
	{
	case eCursorVisual::ARROW:			cursorHandle = LoadCursor(nullptr, IDC_ARROW);			break;
	case eCursorVisual::IBEAM:			cursorHandle = LoadCursor(nullptr, IDC_IBEAM);			break;
	case eCursorVisual::WAIT:			cursorHandle = LoadCursor(nullptr, IDC_WAIT);			break;
	case eCursorVisual::CROSS:			cursorHandle = LoadCursor(nullptr, IDC_CROSS);			break;
	case eCursorVisual::UPARROW:		cursorHandle = LoadCursor(nullptr, IDC_UPARROW);		break;
	case eCursorVisual::SIZE:			cursorHandle = LoadCursor(nullptr, IDC_SIZE);			break;
	case eCursorVisual::ICON:			cursorHandle = LoadCursor(nullptr, IDC_ICON);			break;
	case eCursorVisual::SIZENWSE:		cursorHandle = LoadCursor(nullptr, IDC_SIZENWSE);		break;
	case eCursorVisual::SIZENESW:		cursorHandle = LoadCursor(nullptr, IDC_SIZENESW);		break;
	case eCursorVisual::SIZEWE:			cursorHandle = LoadCursor(nullptr, IDC_SIZEWE);			break;
	case eCursorVisual::SIZENS:			cursorHandle = LoadCursor(nullptr, IDC_SIZENS);			break;
	case eCursorVisual::SIZEALL:		cursorHandle = LoadCursor(nullptr, IDC_SIZEALL);		break;
	case eCursorVisual::NO:				cursorHandle = LoadCursor(nullptr, IDC_NO);				break;
	case eCursorVisual::HAND:			cursorHandle = LoadCursor(nullptr, IDC_HAND);			break;
	case eCursorVisual::APPSTARTING:	cursorHandle = LoadCursor(nullptr, IDC_APPSTARTING);	break;
	case eCursorVisual::HELP:			cursorHandle = LoadCursor(nullptr, IDC_HELP);			break;
	}
	assert(cursorHandle);

	if (hwnd)
		SetClassLong((HWND)hwnd, -12, (DWORD)cursorHandle); // #define GCL_HCURSOR (-12)

	SetCursor(cursorHandle);
}

} // namespace inl